#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "system_manager.h"
#include "tbb_templates.hpp"

namespace ecss {

class Entity {
 public:
  using Internal =
      tbb::concurrent_unordered_map<size_t, tbb::concurrent_vector<std::any>>;
  Entity() : loc_(nullptr) {}
  Entity(int) : loc_(std::make_shared<Internal>()) {}

  bool operator<(const Entity& other) const {
    return loc_.get() < other.loc_.get();
  };
  bool operator>(const Entity& other) const {
    return loc_.get() > other.loc_.get();
  };
  bool operator==(const Entity& other) const {
    return loc_.get() == other.loc_.get();
  };
  bool operator!=(const Entity& other) const {
    return loc_.get() != other.loc_.get();
  };

  std::shared_ptr<Internal> loc_;
};

template <typename Ent>
class EntityManager {
 public:
  template <typename T>
  using Internal = tbb::concurrent_vector<std::pair<T, Ent>>;

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

  EntityManager() {}

  Ent CreateEntity() { return Ent(0); }

  void SyncSwap() {}

  template <typename T>
  T& AddComponent() {
    auto [it, inserted] =
        data_stores_.emplace(typeid(T).hash_code(), std::any(Internal<T>()));
    auto& data_store = std::any_cast<Internal<T>&>(it->second);
    auto element = data_store.emplace_back();
    return element->first;
  }

  template <typename T>
  T* Component() {
    auto [it, inserted] =
        data_stores_.emplace(typeid(T).hash_code(), std::any(Internal<T>()));
    auto& data_store = std::any_cast<Internal<T>&>(it->second);
    if (data_store.empty()) return nullptr;
    return &data_store[0].first;
  }

  template <typename T>
  const T* ComponentR() {
    return Component<T>();
  }

  template <typename T>
  T* ComponentW() {
    return Component<T>();
  }

  template <typename T>
  T& AddComponent(Ent& entity) {
    auto [it, inserted] =
        data_stores_.emplace(typeid(T).hash_code(), std::any(Internal<T>()));
    auto& data_store = std::any_cast<Internal<T>&>(it->second);
    auto element = data_store.emplace_back(T{}, entity);
    auto& loc_map = (*entity.template loc_)[typeid(T).hash_code()];
    loc_map.push_back(std::any(&element->first));
    return element->first;
  }

  template <typename T>
  T* Component(const Ent& entity, std::uint64_t sub_loc = 0) {
    auto ent_it = entity.template loc_->find(typeid(T).hash_code());
    if (ent_it == entity.template loc_->end()) return nullptr;
    auto it = std::any_cast<T*>(ent_it->second[sub_loc]);
    return it;
  }

  template <typename T>
  const T* ComponentR(const Ent& entity, std::uint64_t sub_loc = 0) {
    return Component<T>(entity, sub_loc);
  }

  template <typename T>
  T* ComponentW(const Ent& entity, std::uint64_t sub_loc = 0) {
    return Component<T>(entity, sub_loc);
  }

  template <typename T>
  ComponentHolder<T> Components() {
    auto [it, inserted] =
        data_stores_.emplace(typeid(T).hash_code(), std::any(Internal<T>()));
    auto& data_store = std::any_cast<Internal<T>&>(it->second);
    return ComponentHolder<T>(&data_store);
  }

  template <typename T>
  ComponentHolder<T> ComponentsR() {
    return Components<T>();
  }

  template <typename T>
  ComponentHolder<T> ComponentsW() {
    return Components<T>();
  }

  template <typename T>
  EntityComponents<T> Components(const Ent& entity) {
    auto ent_it = entity.template loc_->find(typeid(T).hash_code());
    if (ent_it == entity.template loc_->end())
      return EntityComponents<T>(nullptr);
    return EntityComponents<T>(&ent_it->second);
  }

  template <typename T>
  EntityComponents<T> ComponentsR(const Ent& entity) {
    return Components<T>(entity);
  }

  template <typename T>
  EntityComponents<T> ComponentsW(const Ent& entity) {
    return Components<T>(entity);
  }

 private:
  tbb::concurrent_unordered_map<size_t, std::any> data_stores_;
};

using Entity_t = Entity;
using EntityManager_t = ecss::EntityManager<ecss::Entity>;
using SystemManager_t = ecs::SystemManager<EntityManager_t>;
}  // namespace ecss
