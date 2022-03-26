#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "entity.h"
#include "system_manager.h"
#include "tbb_templates.hpp"

#ifdef UNIT_TEST
#include "entity_manager_mock.h"
#endif

namespace ecss {

template <typename T>
class EntityComponents {
 public:
  EntityComponents(tbb::concurrent_vector<std::any>* content)
      : content_(content) {}
  EntityComponents& operator=(const EntityComponents& copy) = delete;

  class iterator {
   public:
    iterator() {}
    iterator(tbb::concurrent_vector<std::any>::iterator it) : it_(it) {}

    auto operator++() {
      it_++;
      return *this;
    }

    bool operator!=(const iterator& other) { return other.it_ != it_; }

    auto& operator*() {
      auto entry = std::any_cast<T*>(*it_);
      return *entry;
    }

   private:
    tbb::concurrent_vector<std::any>::iterator it_;
  };

  auto begin() {
    if (content_) return iterator(std::begin(*content_));
    return iterator();
  }

  auto end() {
    if (content_) return iterator(std::end(*content_));
    return iterator();
  }

  auto size() {
    if (content_) return content_->size();
    return size_t(0);
  }

  auto empty() {
    if (content_) return content_->empty();
    return true;
  }

  auto operator[](size_t i) {
    auto entry = std::any_cast<T*>((*content_)[i]);
    return *entry;
  }

 private:
  tbb::concurrent_vector<std::any>* content_;
};

template <typename T>
using Internal = tbb::concurrent_vector<std::pair<T, Entity>>;

template <typename T>
class ComponentHolder {
 public:
  ComponentHolder(Internal<T>* content) : content_(content) {}
  ComponentHolder& operator=(const ComponentHolder& copy) = delete;

  class iterator {
   public:
    iterator() {}
    iterator(typename Internal<T>::iterator it) : it_(it) {}

    auto operator++() {
      it_++;
      return *this;
    }

    bool operator!=(const iterator& other) { return other.it_ != it_; }

    auto operator*() {
      return std::make_tuple(std::ref(it_->first), std::ref(it_->second));
    }

   private:
    typename Internal<T>::iterator it_;
  };

  auto begin() {
    if (content_) return iterator(std::begin(*content_));
    return iterator();
  }

  auto end() {
    if (content_) return iterator(std::end(*content_));
    return iterator();
  }

  auto size() {
    if (content_) return content_->size();
    return size_t(0);
  }

  auto empty() {
    if (content_) return content_->empty();
    return true;
  }

  auto operator[](size_t i) {
    auto& entry = (*content_)[i];
    return std::make_tuple(std::ref(entry.first), std::ref(entry.second));
  }

 private:
  Internal<T>* content_;
};

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
    loc_map.push_back(std::any(&element->first));
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
    auto it = std::any_cast<T*>(ent_it->second[sub_loc]);
    return it;
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
  }

 private:
  tbb::concurrent_unordered_map<size_t, std::any> data_stores_;
};

using Entity_t = Entity;
using EntityManager_t = ecss::EntityManager;
using SystemManager_t = ecs::SystemManager<EntityManager_t>;
}  // namespace ecss
