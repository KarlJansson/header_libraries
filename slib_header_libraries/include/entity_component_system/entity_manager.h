#pragma once

#include <any>
#include <atomic>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "entity.h"
#include "entity_manager_util.h"
#include "system_manager.h"
#include "tbb_templates.hpp"

#ifdef UNIT_TEST
#include "entity_manager_mock.h"
#endif

namespace ecs {
class EntityManager {
 public:
  template <typename T>
  using dsm = std::unordered_map<std::type_index, T>;

#ifdef UNIT_TEST
  EntityManager(EntityManagerMock* mock) : mock_(mock) {}
  EntityManagerMock* mock_;
#endif

  EntityManager() {}

  Entity CreateEntity() {
#ifdef UNIT_TEST
    if (mock_) return std::any_cast<Entity>(mock_->CreateEntity());
#endif
    return Entity(0);
  }

  void SyncSwap() {
#ifdef UNIT_TEST
    if (mock_) return mock_->SyncSwap();
#endif

    if (data_store_updates_.size() < 20)
      for (auto& f : data_store_updates_) f();
    else
      tbb_templates::parallel_for(
          data_store_updates_, [this](size_t i) { data_store_updates_[i](); });

    while (!remove_component_.empty()) {
      (remove_component_.back())();
      remove_component_.pop_back();
    }

    for (auto& entry : add_component_cache_) entry();
    add_component_cache_.clear();

    for (auto& [rem, rem_func] : remove_component_cache_) {
      remove_component_.emplace_back(rem);
      rem_func();
    }
    remove_component_cache_.clear();

    write_buffer_id_ = write_buffer_id_ == 0 ? 1 : 0;
  }

  template <typename T>
  T& AddComponent() {
#ifdef UNIT_TEST
    if (mock_) return std::any_cast<T&>(mock_->AddComponent(typeid(T)));
#endif
    auto ptr = std::make_shared<T>();
    add_component_cache_.push_back([this, ptr]() {
      auto it = data_stores_.find(typeid(T));
      if (it == std::end(data_stores_)) {
        auto data_store = std::make_shared<DataStore<T>>();
        data_stores_.emplace(typeid(T), std::any(data_store));

        data_store->dirty_components.insert(data_store->components[0].size());
        data_store->added_components.emplace_back(
            data_store->components[0].size());
        data_store->entities.emplace_back(0);
        (*data_store->entities.back().loc_map_)[typeid(T)].emplace_back(0);
        data_store->components[0].emplace_back(*ptr);
        data_store->components[1].emplace_back(*ptr);
        data_store_updates_.emplace_back([this]() { UpdateDatastore<T>(); });
      } else {
        auto data_store =
            std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
        data_store->dirty_components.insert(0);
        data_store->added_components.emplace_back(0);
        data_store->components[0][0] = *ptr;
        data_store->components[1][0] = *ptr;
      }
    });
    return *ptr;
  }

  template <typename T>
  const T* ComponentR() const {
#ifdef UNIT_TEST
    if (mock_) return &std::any_cast<T&>(mock_->ComponentR(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_))
      return &std::any_cast<std::shared_ptr<DataStore<T>>>(it->second)
                  ->components[write_buffer_id_ == 0 ? 1 : 0][0];
    return nullptr;
  }

  template <typename T>
  T* ComponentW() {
#ifdef UNIT_TEST
    if (mock_) return &std::any_cast<T&>(mock_->ComponentW(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto data_store =
          std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      data_store->dirty_components.insert(0);
      return &data_store->components[write_buffer_id_][0];
    }
    return nullptr;
  }

  template <typename T>
  ConstComponents<T, Entity> ComponentsR() const {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<ConstComponents<T, Entity>>(
          mock_->ComponentsR(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return ConstComponents<T, Entity>(
          &ds->components[write_buffer_id_ == 0 ? 1 : 0], &ds->entities);
    }
    return ConstComponents<T, Entity>(nullptr, nullptr);
  }

  template <typename T>
  Components<T, Entity> ComponentsW() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<Components<T, Entity>>(
          mock_->ComponentsW(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return Components<T, Entity>(&ds->components[write_buffer_id_],
                                   &ds->entities);
    }
    return Components<T, Entity>(nullptr, nullptr);
  }

  template <typename T>
  UpdatedComponents<const std::vector<T>*, Entity> UpdatedComponentsR() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<UpdatedComponents<const std::vector<T>*, Entity>>(
          mock_->UpdatedComponentsR(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return UpdatedComponents<const std::vector<T>*, Entity>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->updated_components);
    }
    return UpdatedComponents<const std::vector<T>*, Entity>(nullptr, nullptr,
                                                            nullptr);
  }

  template <typename T>
  UpdatedComponents<std::vector<T>*, Entity> UpdatedComponentsW() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<UpdatedComponents<std::vector<T>*, Entity>>(
          mock_->UpdatedComponentsW(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return UpdatedComponents<std::vector<T>*, Entity>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->updated_components);
    }
    return UpdatedComponents<std::vector<T>*, Entity>(nullptr, nullptr,
                                                      nullptr);
  }

  template <typename T>
  UpdatedComponents<const std::vector<T>*, Entity> AddedComponentsR() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<UpdatedComponents<const std::vector<T>*, Entity>>(
          mock_->AddedComponentsR(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return UpdatedComponents<const std::vector<T>*, Entity>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->added_components);
    }
    return UpdatedComponents<const std::vector<T>*, Entity>(nullptr, nullptr,
                                                            nullptr);
  }

  template <typename T>
  UpdatedComponents<std::vector<T>*, Entity> AddedComponentsW() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<UpdatedComponents<std::vector<T>*, Entity>>(
          mock_->AddedComponentsW(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return UpdatedComponents<std::vector<T>*, Entity>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->added_components);
    }
    return UpdatedComponents<std::vector<T>*, Entity>(nullptr, nullptr,
                                                      nullptr);
  }

  template <typename T>
  RemovedComponentsHolder<T, Entity> RemovedComponents() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<RemovedComponentsHolder<T, Entity>>(
          mock_->RemovedComponents(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return RemovedComponentsHolder<T, Entity>(
          &ds->components[write_buffer_id_ == 0 ? 1 : 0], &ds->entities,
          &ds->removed_components);
    }
    return RemovedComponentsHolder<T, Entity>(nullptr, nullptr, nullptr);
  }

  template <typename T>
  EntityHolder<Entity> Entities() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<EntityHolder<Entity>>(mock_->Entities(typeid(T)));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return EntityHolder<Entity>(&ds->entities);
    }
    return EntityHolder<Entity>(nullptr);
  }

  template <typename T>
  T& AddComponent(Entity& entity) {
#ifdef UNIT_TEST
    if (mock_) return std::any_cast<T&>(mock_->AddComponent(typeid(T), entity));
#endif
    auto ptr = std::make_shared<T>();
    add_component_cache_.push_back([this, ptr, entity]() {
      auto it = data_stores_.find(typeid(T));
      if (it == std::end(data_stores_)) {
        auto data_store = std::make_shared<DataStore<T>>();
        data_stores_.emplace(typeid(T), std::any(data_store));

        (*entity.loc_map_)[typeid(T)].push_back(data_store->entities.size());
        data_store->dirty_components.insert(data_store->components[0].size());
        data_store->added_components.emplace_back(
            data_store->components[0].size());
        data_store->entities.emplace_back(entity);
        data_store->components[0].emplace_back(*ptr);
        data_store->components[1].emplace_back(*ptr);
        data_store_updates_.emplace_back([this]() { UpdateDatastore<T>(); });
      } else {
        auto data_store =
            std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
        (*entity.loc_map_)[typeid(T)].push_back(data_store->entities.size());
        data_store->dirty_components.insert(data_store->components[0].size());
        data_store->added_components.emplace_back(
            data_store->components[0].size());
        data_store->entities.emplace_back(entity);
        data_store->components[0].emplace_back(*ptr);
        data_store->components[1].emplace_back(*ptr);
      }
    });
    return *ptr;
  }

  template <typename T>
  void RemoveComponent(Entity& entity, std::uint64_t sub_loc = 0) {
#ifdef UNIT_TEST
    if (mock_) return mock_->RemoveComponent(typeid(T), entity, sub_loc);
#endif
    remove_component_cache_.push_back(std::make_pair(
        [this, entity, sub_loc]() {
          auto ent_loc = entity.Loc<T>(sub_loc);
          if (ent_loc == std::numeric_limits<std::uint64_t>::max()) return;

          auto& loc_map = (*entity.loc_map_)[typeid(T)];
          loc_map.erase(std::begin(loc_map) + sub_loc);

          if (auto ds_it = data_stores_.find(typeid(T));
              ds_it != std::end(data_stores_)) {
            auto data_store =
                std::any_cast<std::shared_ptr<DataStore<T>>>(ds_it->second);

            std::swap(data_store->entities[ent_loc],
                      data_store->entities.back());
            std::swap(data_store->components[0][ent_loc],
                      data_store->components[0].back());
            std::swap(data_store->components[1][ent_loc],
                      data_store->components[1].back());
            data_store->entities.pop_back();
            data_store->components[0].pop_back();
            data_store->components[1].pop_back();
            data_store->removed_components.clear();

            if (!data_store->entities.empty())
              for (auto& e :
                   (*data_store->entities[ent_loc].loc_map_)[typeid(T)])
                if (e == data_store->entities.size()) e = ent_loc;
          }
        },
        [this, entity, sub_loc]() {
          if (auto ds_it = data_stores_.find(typeid(T));
              ds_it != std::end(data_stores_)) {
            auto ent_loc = entity.Loc<T>(sub_loc);
            if (ent_loc == std::numeric_limits<std::uint64_t>::max()) return;

            auto data_store =
                std::any_cast<std::shared_ptr<DataStore<T>>>(ds_it->second);
            data_store->removed_components.emplace_back(ent_loc);
          }
        }));
  }

  template <typename T>
  const T* ComponentR(const Entity& entity, std::uint64_t sub_loc = 0) {
#ifdef UNIT_TEST
    if (mock_)
      return &std::any_cast<T&>(mock_->ComponentR(typeid(T), entity, sub_loc));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ent_loc = entity.Loc<T>(sub_loc);
      if (ent_loc == std::numeric_limits<std::uint64_t>::max()) return nullptr;
      return &std::any_cast<std::shared_ptr<DataStore<T>>>(it->second)
                  ->components[write_buffer_id_ == 0 ? 1 : 0][ent_loc];
    }
    return nullptr;
  }

  template <typename T>
  T* ComponentW(const Entity& entity, std::uint64_t sub_loc = 0) {
#ifdef UNIT_TEST
    if (mock_)
      return &std::any_cast<T&>(mock_->ComponentW(typeid(T), entity, sub_loc));
#endif
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ent_loc = entity.Loc<T>(sub_loc);
      if (ent_loc == std::numeric_limits<std::uint64_t>::max()) return nullptr;
      auto data_store =
          std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      data_store->dirty_components.insert(ent_loc);
      return &data_store->components[write_buffer_id_][ent_loc];
    }
    return nullptr;
  }

  template <typename T>
  EntityComponents<const T> ComponentsR(const Entity& entity) {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<EntityComponents<const T>>(
          mock_->ComponentsR(typeid(T), entity));
#endif
    auto func = [ this, entity ](size_t sub_loc) -> auto {
      return ComponentR<T>(entity, sub_loc);
    };
    return EntityComponents<const T>(ComponentCount<T>(entity), func);
  }

  template <typename T>
  EntityComponents<T> ComponentsW(Entity& entity) {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<EntityComponents<T>>(
          mock_->ComponentsW(typeid(T), entity));
#endif
    auto func = [ this, entity ](size_t sub_loc) -> auto {
      return ComponentW<T>(entity, sub_loc);
    };
    return EntityComponents<T>(ComponentCount<T>(entity), func);
  }

  template <typename T>
  std::uint64_t ComponentCount(const Entity& entity) const {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<std::uint64_t>(
          mock_->ComponentCount(typeid(T), entity));
#endif
    if (auto it = entity.loc_map_->find(typeid(T));
        it != std::end(*entity.loc_map_))
      return it->second.size();
    return std::uint64_t(0);
  }

 private:
  template <typename T>
  void UpdateDatastore() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto data_store =
          std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);

      std::vector<size_t> dirty_components;
      for (auto& ind : data_store->dirty_components)
        dirty_components.emplace_back(ind);
      data_store->dirty_components.clear();

      auto sort_unique = [](auto& vec) {
        std::sort(std::begin(vec), std::end(vec));
        vec.erase(std::unique(std::begin(vec), std::end(vec)), std::end(vec));
      };
      sort_unique(dirty_components);

      auto read_buffer_id = write_buffer_id_ == 0 ? 1 : 0;
      for (auto ind : dirty_components)
        data_store->components[read_buffer_id][ind] =
            data_store->components[write_buffer_id_][ind];
      data_store->updated_components.swap(dirty_components);
      data_store->added_components.clear();
    }
  }

  template <typename T>
  class DataStore {
   public:
    DataStore() {
      components[0].reserve(128);
      components[1].reserve(128);
      entities.reserve(128);
      removed_components.reserve(128);
      updated_components.reserve(128);
      added_components.reserve(128);
    }

    std::vector<T> components[2];
    std::vector<Entity> entities;

    tbb::concurrent_unordered_set<size_t> dirty_components;
    std::vector<size_t> removed_components;
    std::vector<size_t> updated_components;
    std::vector<size_t> added_components;
  };

  std::uint8_t write_buffer_id_{0};

  dsm<std::any> data_stores_;
  std::vector<std::function<void(void)>> data_store_updates_;

  tbb::concurrent_vector<std::function<void(void)>> add_component_cache_;

  tbb::concurrent_vector<
      std::pair<std::function<void(void)>, std::function<void(void)>>>
      remove_component_cache_;
  std::vector<std::function<void(void)>> remove_component_;

  const std::uint16_t MAX_ADD_PER_CYCLE{1024};
  const std::uint16_t MAX_REMOVE_PER_CYCLE{1024};
};

using Entity_t = Entity;
using EntityManager_t = EntityManager;
using SystemManager_t = SystemManager<EntityManager_t>;
}  // namespace ecs

namespace ecss {
class EntityManager {
 public:
#ifdef UNIT_TEST
  EntityManager(EntityManagerMock* mock) : mock_(mock) {}
  EntityManagerMock* mock_{nullptr};
#endif

  EntityManager() = default;

  Entity CreateEntity() {
#ifdef UNIT_TEST
    if (mock_) return std::any_cast<Entity>(mock_->CreateEntity());
#endif
    return Entity(0);
  }

  void SyncSwap() {
#ifdef UNIT_TEST
    if (mock_) return mock_->SyncSwap();
#endif
  }

  template <typename T>
  T& AddComponent() {
#ifdef UNIT_TEST
    if (mock_) return std::any_cast<T&>(mock_->AddComponent(typeid(T)));
#endif
    auto [it, inserted] =
        data_stores_.emplace(typeid(T).hash_code(), std::any(Internal<T>()));
    auto& data_store = std::any_cast<Internal<T>&>(it->second);
    auto element = data_store.emplace_back();
    return element->first;
  }

  template <typename T>
  T* Component() {
#ifdef UNIT_TEST
    if (mock_) return &std::any_cast<T&>(mock_->Component(typeid(T)));
#endif
    auto [it, inserted] =
        data_stores_.emplace(typeid(T).hash_code(), std::any(Internal<T>()));
    auto& data_store = std::any_cast<Internal<T>&>(it->second);
    if (data_store.empty()) return nullptr;
    return &data_store[0].first;
  }

  template <typename T>
  const T* ComponentR() {
#ifdef UNIT_TEST
    if (mock_) return &std::any_cast<T&>(mock_->ComponentR(typeid(T)));
#endif
    return Component<T>();
  }

  template <typename T>
  T* ComponentW() {
#ifdef UNIT_TEST
    if (mock_) return &std::any_cast<T&>(mock_->ComponentW(typeid(T)));
#endif
    return Component<T>();
  }

  template <typename T>
  T& AddComponent(Entity& entity) {
#ifdef UNIT_TEST
    if (mock_) return std::any_cast<T&>(mock_->AddComponent(typeid(T), entity));
#endif
    auto [it, inserted] =
        data_stores_.emplace(typeid(T).hash_code(), std::any(Internal<T>()));
    auto& data_store = std::any_cast<Internal<T>&>(it->second);
    auto element = data_store.emplace_back(T{}, entity);
    auto& loc_map = (*entity.loc_)[typeid(T).hash_code()];
    loc_map.push_back(std::any(element));
    return element->first;
  }

  template <typename T>
  T* Component(const Entity& entity, std::uint64_t sub_loc = 0) {
#ifdef UNIT_TEST
    if (mock_)
      return &std::any_cast<T&>(mock_->Component(typeid(T), entity, sub_loc));
#endif
    auto ent_it = entity.loc_->find(typeid(T).hash_code());
    if (ent_it == entity.loc_->end()) return nullptr;
    auto& it = std::any_cast<
        typename tbb::concurrent_vector<std::pair<T, Entity>>::iterator&>(
        ent_it->second[sub_loc]);
    return &it->first;
  }

  template <typename T>
  const T* ComponentR(const Entity& entity, std::uint64_t sub_loc = 0) {
#ifdef UNIT_TEST
    if (mock_)
      return &std::any_cast<T&>(mock_->ComponentR(typeid(T), entity, sub_loc));
#endif
    return Component<T>(entity, sub_loc);
  }

  template <typename T>
  T* ComponentW(const Entity& entity, std::uint64_t sub_loc = 0) {
#ifdef UNIT_TEST
    if (mock_)
      return &std::any_cast<T&>(mock_->ComponentW(typeid(T), entity, sub_loc));
#endif
    return Component<T>(entity, sub_loc);
  }

  template <typename T>
  ComponentHolder<T> Components() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<ComponentHolder<T>>(mock_->Components(typeid(T)));
#endif
    auto [it, inserted] =
        data_stores_.emplace(typeid(T).hash_code(), std::any(Internal<T>()));
    auto& data_store = std::any_cast<Internal<T>&>(it->second);
    return ComponentHolder<T>(&data_store);
  }

  template <typename T>
  ComponentHolder<T> ComponentsR() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<ComponentHolder<T>>(mock_->ComponentsR(typeid(T)));
#endif
    return Components<T>();
  }

  template <typename T>
  ComponentHolder<T> ComponentsW() {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<ComponentHolder<T>>(mock_->ComponentsW(typeid(T)));
#endif
    return Components<T>();
  }

  template <typename T>
  EntityComponents<T> Components(const Entity& entity) {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<EntityComponents<T>>(
          mock_->Components(typeid(T), entity));
#endif
    auto ent_it = entity.loc_->find(typeid(T).hash_code());
    if (ent_it == entity.loc_->end()) return EntityComponents<T>(nullptr);
    return EntityComponents<T>(&ent_it->second);
  }

  template <typename T>
  EntityComponents<T> ComponentsR(const Entity& entity) {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<EntityComponents<T>>(
          mock_->ComponentsR(typeid(T), entity));
#endif
    return Components<T>(entity);
  }

  template <typename T>
  EntityComponents<T> ComponentsW(const Entity& entity) {
#ifdef UNIT_TEST
    if (mock_)
      return std::any_cast<EntityComponents<T>>(
          mock_->ComponentsW(typeid(T), entity));
#endif
    return Components<T>(entity);
  }

  template <typename T>
  void RemoveComponent() {
#ifdef UNIT_TEST
    if (mock_) return mock_->RemoveComponent(typeid(T));
#endif
  }

  template <typename T>
  void RemoveComponent(const Entity& entity, const std::uint64_t sub_loc = 0) {
#ifdef UNIT_TEST
    if (mock_) return mock_->RemoveComponent(typeid(T), entity, sub_loc);
#endif
    auto ent_it = entity.loc_->find(typeid(T).hash_code());
    if (ent_it == entity.loc_->end()) return;
    auto& it = std::any_cast<
        typename tbb::concurrent_vector<std::pair<T, Entity>>::iterator&>(
        ent_it->second[sub_loc]);
    it->second = Entity();
  }

 private:
  tbb::concurrent_unordered_map<size_t, std::any> data_stores_;
};

using Entity_t = Entity;
using EntityManager_t = ecss::EntityManager;
using SystemManager_t = ecs::SystemManager<EntityManager_t>;
}  // namespace ecss
