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

#include "tbb_templates.hpp"
#include "system_manager.h"

namespace ecs {
template <typename T>
using dsm = std::unordered_map<std::type_index, T>;

class Entity {
 public:
  Entity() : loc_map_(nullptr) {}
  Entity(int) : loc_map_(std::make_shared<dsm<std::vector<std::uint64_t>>>()) {}

  bool operator<(const Entity& other) const {
    return loc_map_.get() < other.loc_map_.get();
  };
  bool operator>(const Entity& other) const {
    return loc_map_.get() > other.loc_map_.get();
  };
  bool operator==(const Entity& other) const {
    return loc_map_.get() == other.loc_map_.get();
  };
  bool operator!=(const Entity& other) const {
    return loc_map_.get() != other.loc_map_.get();
  };

  template <typename T>
  std::uint64_t Loc(std::uint64_t sub_loc = 0) const {
    if (auto it = loc_map_->find(typeid(T));
        it != std::end(*loc_map_) && it->second.size() > sub_loc)
      return std::get<std::vector<std::uint64_t>>(*it)[sub_loc];
    return std::numeric_limits<std::uint64_t>::max();
  }

  std::shared_ptr<dsm<std::vector<std::uint64_t>>> loc_map_;
};

template <typename T>
class EntityComponents {
 public:
  EntityComponents(size_t size, std::function<T*(size_t)> func)
      : size_(size), func(func) {}
  EntityComponents& operator=(const EntityComponents& copy) = delete;

  class iterator {
   public:
    iterator(size_t sub_loc, std::function<T*(size_t)> func)
        : sub_loc(sub_loc), func(func) {}

    auto operator++() {
      ++sub_loc;
      return *this;
    }
    bool operator!=(const iterator& other) { return other.sub_loc != sub_loc; }
    auto& operator*() { return *func(sub_loc); }

   private:
    size_t sub_loc;
    std::function<T*(size_t)> func;
  };

  auto begin() { return iterator(0, func); }
  auto end() { return iterator(size_, func); }

  auto size() { return size_; }
  auto empty() { return size_ == 0; }

  auto operator[](size_t i) { return *func(i); }

 private:
  size_t size_;
  std::function<T*(size_t)> func;
};

template <typename T, typename Ent>
class RemovedComponentsHolder {
 public:
  RemovedComponentsHolder(const std::vector<T>* comps, std::vector<Ent>* ents,
                          std::vector<size_t>* comp_locs)
      : components(comps), entities(ents), component_locs(comp_locs) {}
  RemovedComponentsHolder& operator=(const RemovedComponentsHolder& copy) =
      delete;

  template <typename T1>
  class iterator {
   public:
    iterator(T1 comps, std::vector<Ent>* ents, size_t* loc)
        : components(comps), entities(ents), loc(loc) {}

    auto operator++() {
      loc++;
      return *this;
    }

    bool operator!=(const iterator& other) { return other.loc != loc; }

    auto operator*() {
      return std::make_tuple(std::ref(components[*loc]),
                             std::ref(entities[*loc]));
    }

   private:
    T1 components;
    std::vector<Ent>* entities;
    size_t* loc;
  };

  auto begin() {
    if (components)
      return iterator(components, entities, component_locs->data());
    return iterator<const std::vector<T>*>(nullptr, nullptr, nullptr);
  }

  auto end() {
    if (components)
      return iterator(components, entities,
                      component_locs->data() + component_locs->size());
    return iterator<const std::vector<T>*>(nullptr, nullptr, nullptr);
  }

  auto size() {
    if (component_locs) return component_locs->size();
    return std::size_t(0);
  }

  auto empty() {
    if (component_locs) return component_locs->size() == 0;
    return true;
  }

  auto operator[](size_t i) {
    return std::make_tuple(std::ref(components[(*component_locs)[i]]),
                           std::ref(entities[(*component_locs)[i]]));
  }

  const std::vector<T>* components;
  std::vector<Ent>* entities;
  std::vector<size_t>* component_locs;
};

template <typename Ent>
class EntityHolder {
 public:
  EntityHolder(std::vector<Ent>* ents) : entities(ents) {}
  EntityHolder& operator=(const EntityHolder& copy) = delete;

  class iterator {
   public:
    iterator(Ent* ent) : entity(ent) {}

    auto operator++() {
      ++entity;
      return *this;
    }

    bool operator!=(const iterator& other) { return other.entity != entity; }

    auto operator*() { return *entity; }

   private:
    Ent* entity;
  };

  auto begin() {
    if (entities) return iterator(entities->data());
    return iterator(nullptr);
  }

  auto end() {
    if (entities) return iterator(entities->data() + entities->size());
    return iterator(nullptr);
  }

  auto size() {
    if (entities) return entities->size();
    return std::size_t(0);
  }

  auto empty() {
    if (entities) return entities->size() == 0;
    return true;
  }

  auto operator[](size_t i) { return (*entities)[i]; }

  std::vector<Ent>* entities{nullptr};
};

template <typename T, typename Ent>
class Components {
 public:
  Components(std::vector<T>* comps, std::vector<Ent>* ents)
      : components(comps), entities(ents) {}
  Components& operator=(const Components& copy) = delete;

  template <typename T1, typename T2>
  class iterator {
   public:
    iterator(T1* comp, T2* ent) : component(comp), entity(ent) {}

    auto operator++() {
      ++component;
      ++entity;
      return *this;
    }
    bool operator!=(const iterator& other) {
      return other.component != component;
    }
    auto operator*() {
      return std::make_tuple(std::ref(*component), std::ref(*entity));
    }

   private:
    T1* component;
    T2* entity;
  };

  auto begin() {
    if (components) return iterator(components->data(), entities->data());
    return iterator<T, Ent>(nullptr, nullptr);
  }

  auto end() {
    if (components)
      return iterator(components->data() + components->size(),
                      entities->data() + entities->size());
    return iterator<T, Ent>(nullptr, nullptr);
  }

  auto size() {
    if (components && entities)
      return std::min(components->size(), entities->size());
    return std::size_t(0);
  }

  auto empty() {
    if (components && entities)
      return std::min(components->size(), entities->size()) == 0;
    return true;
  }

  auto operator[](size_t i) {
    return std::make_tuple(std::ref((*components)[i]),
                           std::ref((*entities)[i]));
  }

  std::vector<T>* components{nullptr};
  std::vector<Ent>* entities{nullptr};
};

template <typename T, typename Ent>
class ConstComponents {
 public:
  ConstComponents(const std::vector<T>* comps, std::vector<Ent>* ents)
      : components(comps), entities(ents) {}
  ConstComponents& operator=(const ConstComponents& copy) = delete;

  template <typename T1, typename T2>
  class iterator {
   public:
    iterator(T1* comp, T2* ent) : component(comp), entity(ent) {}

    auto operator++() {
      ++component;
      ++entity;
      return *this;
    }
    bool operator!=(const iterator& other) {
      return other.component != component;
    }
    auto operator*() {
      return std::make_tuple(std::ref(*component), std::ref(*entity));
    }

   private:
    T1* component;
    T2* entity;
  };

  auto begin() {
    if (components) return iterator(components->data(), entities->data());
    return iterator<const T, Ent>(nullptr, nullptr);
  }

  auto end() {
    if (components)
      return iterator(components->data() + components->size(),
                      entities->data() + entities->size());
    return iterator<const T, Ent>(nullptr, nullptr);
  }

  auto size() {
    if (components && entities)
      return std::min(components->size(), entities->size());
    return std::size_t(0);
  }

  auto empty() {
    if (components && entities)
      return std::min(components->size(), entities->size()) == 0;
    return true;
  }

  auto operator[](size_t i) {
    return std::make_tuple(std::ref((*components)[i]),
                           std::ref((*entities)[i]));
  }

  const std::vector<T>* components{nullptr};
  std::vector<Ent>* entities{nullptr};
};

template <typename T, typename Ent>
class UpdatedComponents {
 public:
  UpdatedComponents(T comps, std::vector<Ent>* ents, std::vector<size_t>* inds)
      : components(comps), entities(ents), indices(inds) {}
  UpdatedComponents& operator=(const UpdatedComponents& copy) = delete;

  template <typename T1, typename T2>
  class iterator {
   public:
    iterator(T1 comp, T2 ent, size_t* inds)
        : component(comp), entity(ent), ind_iterator(inds) {}

    auto operator++() {
      ind_iterator++;
      return *this;
    }

    bool operator!=(const iterator& other) {
      return other.ind_iterator != ind_iterator;
    }

    auto operator*() {
      return std::make_tuple(std::ref((*component)[*ind_iterator]),
                             std::ref((*entity)[*ind_iterator]));
    }

   private:
    T1 component;
    T2 entity;
    size_t* ind_iterator;
  };

  auto begin() {
    if (components) return iterator(components, entities, indices->data());
    return iterator<T, std::vector<Ent>*>(nullptr, nullptr, nullptr);
  }

  auto end() {
    if (components)
      return iterator(components, entities, indices->data() + indices->size());
    return iterator<T, std::vector<Ent>*>(nullptr, nullptr, nullptr);
  }

  auto size() {
    if (indices) return indices->size();
    return std::size_t(0);
  }

  auto empty() {
    if (indices) return indices->size() == 0;
    return true;
  }

  auto operator[](size_t i) {
    return std::make_tuple(std::ref((*components)[(*indices)[i]]),
                           std::ref((*entities)[(*indices)[i]]));
  }

  T components;
  std::vector<Ent>* entities;
  std::vector<size_t>* indices;
};

class EntityManager {
 public:
  EntityManager() {}

  Entity CreateEntity() { return Entity(0); }

  void SyncSwap() {
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
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_))
      return &std::any_cast<std::shared_ptr<DataStore<T>>>(it->second)
                  ->components[write_buffer_id_ == 0 ? 1 : 0][0];
    return nullptr;
  }

  template <typename T>
  T* ComponentW() {
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
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return ConstComponents<T, Entity>(
          &ds->components[write_buffer_id_ == 0 ? 1 : 0], &ds->entities);
    }
    return ConstComponents<T, Entity>(nullptr, nullptr);
  }

  template <typename T>
  Components<T, Entity> ComponentsW() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return Components<T, Entity>(&ds->components[write_buffer_id_],
                                &ds->entities);
    }
    return Components<T, Entity>(nullptr, nullptr);
  }

  template <typename T>
  UpdatedComponents<const std::vector<T>*, Entity> UpdatedComponentsR() {
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
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return UpdatedComponents<std::vector<T>*, Entity>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->updated_components);
    }
    return UpdatedComponents<std::vector<T>*, Entity>(nullptr, nullptr, nullptr);
  }

  template <typename T>
  UpdatedComponents<const std::vector<T>*, Entity> AddedComponentsR() {
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
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return UpdatedComponents<std::vector<T>*, Entity>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->added_components);
    }
    return UpdatedComponents<std::vector<T>*, Entity>(nullptr, nullptr, nullptr);
  }

  template <typename T>
  RemovedComponentsHolder<T, Entity> RemovedComponents() {
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
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T>>>(it->second);
      return EntityHolder<Entity>(&ds->entities);
    }
    return EntityHolder<Entity>(nullptr);
  }

  template <typename T>
  T& AddComponent(Entity& entity) {
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
  std::uint64_t ComponentCount(const Entity& entity) const {
    if (auto it = entity.loc_map_->find(typeid(T));
        it != std::end(*entity.loc_map_))
      return it->second.size();
    return std::uint64_t(0);
  }

  template <typename T>
  EntityComponents<const T> ComponentsR(const Entity& entity) {
    auto func = [this, entity](size_t sub_loc) -> auto {
      return ComponentR<T>(entity, sub_loc);
    };
    return EntityComponents<const T>(ComponentCount<T>(entity), func);
  }

  template <typename T>
  EntityComponents<T> ComponentsW(Entity& entity) {
    auto func = [this, entity](size_t sub_loc) -> auto {
      return ComponentW<T>(entity, sub_loc);
    };
    return EntityComponents<T>(ComponentCount<T>(entity), func);
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
