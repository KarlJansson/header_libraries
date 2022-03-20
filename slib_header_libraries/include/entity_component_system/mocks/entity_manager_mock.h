#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "entity.h"
#include "entity_manager.h"

namespace ecs {
#define REGISTER_COMPONENT_TYPE(type)                                          \
  MOCK_METHOD(const type*, ComponentR, (type), (const));                       \
  MOCK_METHOD(const type*, ComponentR,                                         \
              (type, const ecs::Entity&, std::uint64_t), (const));             \
  MOCK_METHOD(type*, ComponentW, (type));                                      \
  MOCK_METHOD(type*, ComponentW, (type, const ecs::Entity&, std::uint64_t));   \
  MOCK_METHOD(type&, AddComponent, (type));                                    \
  MOCK_METHOD(type&, AddComponent, (type, ecs::Entity&));                      \
  MOCK_METHOD((UpdatedComponents<std::vector<type>*, ecs::Entity>),            \
              AddedComponentsW, (type));                                       \
  MOCK_METHOD((UpdatedComponents<const std::vector<type>*, ecs::Entity>),      \
              AddedComponentsR, (type));                                       \
  MOCK_METHOD((UpdatedComponents<std::vector<type>*, ecs::Entity>),            \
              UpdatedComponentsW, (type));                                     \
  MOCK_METHOD((UpdatedComponents<const std::vector<type>*, ecs::Entity>),      \
              UpdatedComponentsR, (type));                                     \
  MOCK_METHOD(void, RemoveComponent, (type));                                  \
  MOCK_METHOD(void, RemoveComponent, (type, ecs::Entity&, std::uint64_t));     \
  MOCK_METHOD((RemovedComponentsHolder<type, ecs::Entity>), RemovedComponents, \
              (type));                                                         \
  MOCK_METHOD((EntityHolder<ecs::Entity>), Entities, (type));                  \
  MOCK_METHOD(void, RegisterType, (type));

class EntityManagerMock {
 public:
  REGISTER_COMPONENT_TYPE(int);
  REGISTER_COMPONENT_TYPE(double);

  MOCK_METHOD(Entity, CreateEntity, ());
  MOCK_METHOD(void, SyncSwap, ());

  template <typename T>
  const T* ComponentR() const {
    return ComponentR(T{});
  }

  template <typename T>
  T* ComponentW() {
    return ComponentW(T{});
  }

  template <typename T>
  const T* ComponentR(Entity& ent, std::uint64_t sub_lock = 0) const {
    return ComponentR(T{}, ent, sub_lock);
  }

  template <typename T>
  T* ComponentW(Entity& ent, std::uint64_t sub_lock = 0) {
    return ComponentW(T{}, ent, sub_lock);
  }

  template <typename T>
  T& AddComponent() {
    return AddComponent(T{});
  }

  template <typename T>
  void RemoveComponent() {
    RemoveComponent(T{});
  }

  template <typename T>
  T* Component(Entity& ent) {
    return Component(T{}, ent);
  }

  template <typename T>
  T& AddComponent(Entity& ent) {
    (*ent.loc_map_)[typeid(T)].emplace_back(0);
    return AddComponent(T{}, ent);
  }

  template <typename T>
  void RemoveComponent(Entity& ent, std::uint64_t sub_lock = 0) {
    (*ent.loc_map_)[typeid(T)].pop_back();
    RemoveComponent(T{}, ent, sub_lock);
  }

  template <typename T>
  std::uint64_t ComponentCount(Entity& entity) const {
    if (auto it = entity.loc_map_->find(typeid(T));
        it != std::end(*entity.loc_map_))
      return it->second.size();
    return std::uint64_t(0);
  }

  template <typename T>
  EntityComponents<const T> ComponentsR(Entity& entity) {
    auto func = [this, entity](size_t sub_loc) -> auto {
      return ComponentR(T{}, entity, sub_loc);
    };
    return EntityComponents<const T>(ComponentCount<T>(entity), func);
  }

  template <typename T>
  EntityComponents<T> ComponentsW(Entity& entity) {
    auto func = [this, entity](size_t sub_loc) -> auto {
      return ComponentW(T{}, entity, sub_loc);
    };
    return EntityComponents<T>(ComponentCount<T>(entity), func);
  }

  template <typename T>
  UpdatedComponents<std::vector<T>*, Entity> AddedComponentsW() {
    return AddedComponentsW(T{});
  }

  template <typename T>
  UpdatedComponents<const std::vector<T>*, Entity> AddedComponentsR() {
    return AddedComponentsR(T{});
  }

  template <typename T>
  UpdatedComponents<std::vector<T>*, Entity> UpdatedComponentsW() {
    return UpdatedComponentsW(T{});
  }

  template <typename T>
  UpdatedComponents<const std::vector<T>*, Entity> UpdatedComponentsR() {
    return UpdatedComponentsR(T{});
  }

  template <typename T>
  RemovedComponentsHolder<T, Entity> RemovedComponents() {
    return RemovedComponents(T{});
  }

  template <typename T>
  EntityHolder<Entity> Entities() {
    return Entities(T{});
  }

  template <typename T>
  void RegisterType() {
    RegisterType(T{});
  }
};

using MockEntity_t = Entity;
}  // namespace ecs
