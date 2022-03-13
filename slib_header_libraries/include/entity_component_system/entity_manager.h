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

#include "chunk_list.hpp"
#include "entity.h"
#include "tbb_templates.hpp"

namespace ecs {
template <typename T>
using dsm = std::unordered_map<std::type_index, T>;

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
    iterator(T1 comps, std::vector<Ent>* ents,
             std::optional<std::vector<size_t>::iterator> loc)
        : components(comps), entities(ents), loc(loc) {}

    auto operator++() {
      (*loc)++;
      return *this;
    }
    bool operator!=(const iterator& other) {
      if (!other.loc.has_value() || !loc.has_value()) return false;
      return other.loc.value() != loc.value();
    }
    auto operator*() {
      auto location = *loc.value();
      return std::make_tuple(&components->at(location),
                             &entities->at(location));
    }

   private:
    T1 components;
    std::vector<Ent>* entities;
    std::optional<std::vector<size_t>::iterator> loc;
  };

  auto begin() {
    if (components)
      return iterator(components, entities, component_locs->begin());
    return iterator<const std::vector<T>*>(nullptr, nullptr, std::nullopt);
  }
  auto end() {
    if (components)
      return iterator(components, entities, component_locs->end());
    return iterator<const std::vector<T>*>(nullptr, nullptr, std::nullopt);
  }

  const std::vector<T>* components;
  std::vector<Ent>* entities;
  std::vector<size_t>* component_locs;
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
    auto operator*() { return std::tuple(component, entity); }

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

  size_t size() {
    if (components && entities)
      return std::min(components->size(), entities->size());
    return 0;
  }

  auto operator[](size_t i) {
    return std::tuple(&(*components)[i], &(*entities)[i]);
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
    auto operator*() { return std::tuple(component, entity); }

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

  size_t size() {
    if (components && entities)
      return std::min(components->size(), entities->size());
    return 0;
  }

  auto operator[](size_t i) {
    return std::tuple(&(*components)[i], &(*entities)[i]);
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
    iterator(T1 comp, T2 ent, std::optional<std::vector<size_t>::iterator> it)
        : component(comp), entity(ent), ind_iterator(it) {}

    auto operator++() {
      (*ind_iterator)++;
      return *this;
    }
    bool operator!=(const iterator& other) {
      return other.ind_iterator != ind_iterator;
    }
    auto operator*() {
      return std::tuple(&component->at(*ind_iterator.value()),
                        &entity->at(*ind_iterator.value()));
    }

   private:
    T1 component;
    T2 entity;
    std::optional<std::vector<size_t>::iterator> ind_iterator;
  };

  auto begin() {
    if (components) return iterator(components, entities, indices->begin());
    return iterator<T, std::vector<Ent>*>(nullptr, nullptr, std::nullopt);
  }
  auto end() {
    if (components) return iterator(components, entities, indices->end());
    return iterator<T, std::vector<Ent>*>(nullptr, nullptr, std::nullopt);
  }
  auto size() { return indices->size(); }

  T components;
  std::vector<Ent>* entities;
  std::vector<size_t>* indices;
};

class EntityManager {
 public:
  EntityManager() {}

  Entity<EntityManager> CreateEntity() { return Entity<EntityManager>(this); }

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

    std::uint8_t counter{0};
    while (add_component_cache_.back() && counter < MAX_ADD_PER_CYCLE) {
      (*add_component_cache_.back())();
      add_component_cache_.pop_back();
      ++counter;
    }

    counter = 0;
    while (remove_component_cache_.back() && counter < MAX_REMOVE_PER_CYCLE) {
      remove_component_.emplace_back(remove_component_cache_.back()->first);
      (remove_component_cache_.back()->second)();
      remove_component_cache_.pop_back();
      ++counter;
    }

    write_buffer_id_ = write_buffer_id_ == 0 ? 1 : 0;
  }

  template <typename T, typename Ent>
  T* AddComponent() {
    auto ptr = std::make_shared<T>();
    add_component_cache_.push_front([this, ptr]() {
      auto it = data_stores_.find(typeid(T));
      if (it == std::end(data_stores_)) {
        auto data_store = std::make_shared<DataStore<T, Ent>>();
        data_stores_.emplace(typeid(T), std::any(data_store));

        data_store->dirty_components.emplace_back(
            data_store->components[0].size());
        data_store->added_components.emplace_back(
            data_store->components[0].size());
        data_store->entities.emplace_back(this);
        (*data_store->entities.back().loc_map_)[typeid(T)].emplace_back(0);
        data_store->components[0].emplace_back(*ptr);
        data_store->components[1].emplace_back(*ptr);
        data_store_updates_.emplace_back(
            [this]() { UpdateDatastore<T, Ent>(); });
      } else {
        auto data_store =
            std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
        data_store->dirty_components.emplace_back(0);
        data_store->added_components.emplace_back(0);
        data_store->components[0][0] = *ptr;
        data_store->components[1][0] = *ptr;
      }
    });
    return ptr.get();
  }

  template <typename T, typename Ent>
  const T* ComponentR() const {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_))
      return &std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second)
                  ->components[write_buffer_id_ == 0 ? 1 : 0][0];
    return nullptr;
  }

  template <typename T, typename Ent>
  T* ComponentW() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto data_store =
          std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      data_store->dirty_components.emplace_back(0);
      return &data_store->components[write_buffer_id_][0];
    }
    return nullptr;
  }

  template <typename T, typename Ent>
  ConstComponents<T, Ent> ComponentsR() const {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      return ConstComponents<T, Ent>(
          &ds->components[write_buffer_id_ == 0 ? 1 : 0], &ds->entities);
    }
    return ConstComponents<T, Ent>(nullptr, nullptr);
  }

  template <typename T, typename Ent>
  Components<T, Ent> ComponentsW() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      return Components<T, Ent>(&ds->components[write_buffer_id_],
                                &ds->entities);
    }
    return Components<T, Ent>(nullptr, nullptr);
  }

  template <typename T, typename Ent>
  UpdatedComponents<const std::vector<T>*, Ent> UpdatedComponentsR() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      return UpdatedComponents<const std::vector<T>*, Ent>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->updated_components);
    }
    return UpdatedComponents<const std::vector<T>*, Ent>(nullptr, nullptr,
                                                         nullptr);
  }

  template <typename T, typename Ent>
  UpdatedComponents<std::vector<T>*, Ent> UpdatedComponentsW() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      return UpdatedComponents<std::vector<T>*, Ent>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->updated_components);
    }
    return UpdatedComponents<std::vector<T>*, Ent>(nullptr, nullptr, nullptr);
  }

  template <typename T, typename Ent>
  UpdatedComponents<const std::vector<T>*, Ent> AddedComponentsR() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      return UpdatedComponents<const std::vector<T>*, Ent>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->added_components);
    }
    return UpdatedComponents<const std::vector<T>*, Ent>(nullptr, nullptr,
                                                         nullptr);
  }

  template <typename T, typename Ent>
  UpdatedComponents<std::vector<T>*, Ent> AddedComponentsW() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      return UpdatedComponents<std::vector<T>*, Ent>(
          &ds->components[write_buffer_id_], &ds->entities,
          &ds->added_components);
    }
    return UpdatedComponents<std::vector<T>*, Ent>(nullptr, nullptr, nullptr);
  }

  template <typename T, typename Ent>
  RemovedComponentsHolder<T, Ent> RemovedComponents() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ds = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      return RemovedComponentsHolder<T, Ent>(
          &ds->components[write_buffer_id_ == 0 ? 1 : 0], &ds->entities,
          &ds->removed_components);
    }
    return RemovedComponentsHolder<T, Ent>(nullptr, nullptr, nullptr);
  }

  template <typename T, typename Ent>
  std::vector<Ent>* Entities() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_))
      return &std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second)
                  ->entities;
    return nullptr;
  }

  template <typename T, typename Ent>
  T* AddComponent(Ent& entity) {
    auto ptr = std::make_shared<T>();
    add_component_cache_.push_front([this, ptr, entity]() {
      auto it = data_stores_.find(typeid(T));
      if (it == std::end(data_stores_)) {
        auto data_store = std::make_shared<DataStore<T, Ent>>();
        data_stores_.emplace(typeid(T), std::any(data_store));

        (*entity.loc_map_)[typeid(T)].push_back(data_store->entities.size());
        data_store->dirty_components.emplace_back(
            data_store->components[0].size());
        data_store->added_components.emplace_back(
            data_store->components[0].size());
        data_store->entities.emplace_back(entity);
        data_store->components[0].emplace_back(*ptr);
        data_store->components[1].emplace_back(*ptr);
        data_store_updates_.emplace_back(
            [this]() { UpdateDatastore<T, Ent>(); });
      } else {
        auto data_store =
            std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
        (*entity.loc_map_)[typeid(T)].push_back(data_store->entities.size());
        data_store->dirty_components.emplace_back(
            data_store->components[0].size());
        data_store->added_components.emplace_back(
            data_store->components[0].size());
        data_store->entities.emplace_back(entity);
        data_store->components[0].emplace_back(*ptr);
        data_store->components[1].emplace_back(*ptr);
      }
    });
    return ptr.get();
  }

  template <typename T, typename Ent>
  void RemoveComponent(Ent& entity, std::uint64_t sub_loc) {
    remove_component_cache_.push_front(std::make_pair(
        [this, entity, sub_loc]() {
          auto ent_loc = entity.template Loc<T>(sub_loc);
          if (ent_loc == std::numeric_limits<std::uint64_t>::max()) return;

          auto& loc_map = (*entity.loc_map_)[typeid(T)];
          loc_map.erase(std::begin(loc_map) + sub_loc);

          if (auto ds_it = data_stores_.find(typeid(T));
              ds_it != std::end(data_stores_)) {
            auto data_store = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(
                ds_it->second);

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
            auto ent_loc = entity.template Loc<T>(sub_loc);
            if (ent_loc == std::numeric_limits<std::uint64_t>::max()) return;

            auto data_store = std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(
                ds_it->second);
            data_store->removed_components.emplace_back(ent_loc);
          }
        }));
  }

  template <typename T, typename Ent>
  const T* ComponentR(Ent& entity, std::uint64_t sub_loc) {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ent_loc = entity.template Loc<T>(sub_loc);
      if (ent_loc == std::numeric_limits<std::uint64_t>::max()) return nullptr;
      return &std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second)
                  ->components[write_buffer_id_ == 0 ? 1 : 0][ent_loc];
    }
    return nullptr;
  }

  template <typename T, typename Ent>
  T* ComponentW(Ent& entity, std::uint64_t sub_loc) {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto ent_loc = entity.template Loc<T>(sub_loc);
      if (ent_loc == std::numeric_limits<std::uint64_t>::max()) return nullptr;
      auto data_store =
          std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);
      data_store->dirty_components.emplace_back(ent_loc);
      return &data_store->components[write_buffer_id_][ent_loc];
    }
    return nullptr;
  }

 private:
  template <typename T, typename Ent>
  void UpdateDatastore() {
    if (auto it = data_stores_.find(typeid(T)); it != std::end(data_stores_)) {
      auto data_store =
          std::any_cast<std::shared_ptr<DataStore<T, Ent>>>(it->second);

      auto sort_unique = [](auto& vec) {
        std::sort(std::begin(vec), std::end(vec));
        vec.erase(std::unique(std::begin(vec), std::end(vec)), std::end(vec));
      };
      sort_unique(data_store->dirty_components);

      auto read_buffer_id = write_buffer_id_ == 0 ? 1 : 0;
      for (auto ind : data_store->dirty_components)
        data_store->components[read_buffer_id][ind] =
            data_store->components[write_buffer_id_][ind];
      data_store->updated_components.swap(data_store->dirty_components);
      data_store->dirty_components.clear();
      data_store->added_components.clear();
    }
  }

  template <typename T, typename Ent>
  class DataStore {
   public:
    DataStore() {
      components[0].reserve(128);
      components[1].reserve(128);
      entities.reserve(128);
      dirty_components.reserve(128);
      removed_components.reserve(128);
      updated_components.reserve(128);
      added_components.reserve(128);
    }

    std::vector<T> components[2];
    std::vector<Ent> entities;

    std::vector<size_t> dirty_components;
    std::vector<size_t> removed_components;
    std::vector<size_t> updated_components;
    std::vector<size_t> added_components;
  };

  std::uint8_t write_buffer_id_{0};

  dsm<std::any> data_stores_;
  std::vector<std::function<void(void)>> data_store_updates_;

  synchronization::ChunkList<std::function<void(void)>, 1024>
      add_component_cache_;

  synchronization::ChunkList<
      std::pair<std::function<void(void)>, std::function<void(void)>>, 1024>
      remove_component_cache_;
  std::vector<std::function<void(void)>> remove_component_;

  const std::uint16_t MAX_ADD_PER_CYCLE{1024};
  const std::uint16_t MAX_REMOVE_PER_CYCLE{1024};
};

using Entity_t = Entity<EntityManager>;
using EntityManager_t = EntityManager;
}  // namespace ecs
