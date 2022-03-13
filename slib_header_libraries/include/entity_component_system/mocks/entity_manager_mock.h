#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "entity.h"
#include "entity_manager.h"

namespace ecs {
#define REGISTER_COMPONENT_TYPE(type)                                        \
  MOCK_METHOD(const type*, ComponentR, (type), (const));                     \
  MOCK_METHOD(const type*, ComponentR,                                       \
              (type, ecs::Entity<ecs::EntityManagerMock>&, std::uint64_t),   \
              (const));                                                      \
  MOCK_METHOD(type*, ComponentW, (type));                                    \
  MOCK_METHOD(type*, ComponentW,                                             \
              (type, ecs::Entity<ecs::EntityManagerMock>&, std::uint64_t));  \
  MOCK_METHOD(type*, AddComponent, (type));                                  \
  MOCK_METHOD(type*, AddComponent,                                           \
              (type, ecs::Entity<ecs::EntityManagerMock>&));                 \
  MOCK_METHOD((UpdatedComponents<std::vector<type>*,                         \
                                 ecs::Entity<ecs::EntityManagerMock>>),      \
              AddedComponentsW, (type));                                     \
  MOCK_METHOD((UpdatedComponents<const std::vector<type>*,                   \
                                 ecs::Entity<ecs::EntityManagerMock>>),      \
              AddedComponentsR, (type));                                     \
  MOCK_METHOD((UpdatedComponents<std::vector<type>*,                         \
                                 ecs::Entity<ecs::EntityManagerMock>>),      \
              UpdatedComponentsW, (type));                                   \
  MOCK_METHOD((UpdatedComponents<const std::vector<type>*,                   \
                                 ecs::Entity<ecs::EntityManagerMock>>),      \
              UpdatedComponentsR, (type));                                   \
  MOCK_METHOD(void, RemoveComponent, (type));                                \
  MOCK_METHOD(void, RemoveComponent,                                         \
              (type, ecs::Entity<ecs::EntityManagerMock>&, std::uint64_t));  \
  MOCK_METHOD(                                                               \
      (RemovedComponentsHolder<type, ecs::Entity<ecs::EntityManagerMock>>),  \
      RemovedComponents, (type));                                            \
  MOCK_METHOD((std::vector<ecs::Entity<ecs::EntityManagerMock>>*), Entities, \
              (type));

class EntityManagerMock {
 public:
  REGISTER_COMPONENT_TYPE(int);
  REGISTER_COMPONENT_TYPE(double);

  MOCK_METHOD(Entity<EntityManagerMock>, CreateEntity, ());
  MOCK_METHOD(void, SyncSwap, ());

  template <typename T>
  const T* ComponentR() const {
    return ComponentR(T{});
  }

  template <typename T>
  T* ComponentW() {
    return ComponentW(T{});
  }

  template <typename T, typename Ent>
  const T* ComponentR(Ent& ent, std::uint64_t sub_lock) const {
    return ComponentR(T{}, ent, sub_lock);
  }

  template <typename T, typename Ent>
  T* ComponentW(Ent& ent, std::uint64_t sub_lock) {
    return ComponentW(T{}, ent, sub_lock);
  }

  template <typename T>
  T* AddComponent() {
    return AddComponent(T{});
  }

  template <typename T>
  void RemoveComponent() {
    RemoveComponent(T{});
  }

  template <typename T, typename Ent>
  T* Component(Ent& ent) {
    return Component(T{}, ent);
  }

  template <typename T, typename Ent>
  T* AddComponent(Ent& ent) {
    (*ent.loc_map_)[typeid(T)].emplace_back(0);
    return AddComponent(T{}, ent);
  }

  template <typename T, typename Ent>
  void RemoveComponent(Ent& ent, std::uint64_t sub_lock) {
    (*ent.loc_map_)[typeid(T)].pop_back();
    RemoveComponent(T{}, ent, sub_lock);
  }

  template <typename T, typename Ent>
  UpdatedComponents<std::vector<T>*, Ent> AddedComponentsW() {
    return AddedComponentsW(T{});
  }

  template <typename T, typename Ent>
  UpdatedComponents<const std::vector<T>*, Ent> AddedComponentsR() {
    return AddedComponentsR(T{});
  }

  template <typename T, typename Ent>
  UpdatedComponents<std::vector<T>*, Ent> UpdatedComponentsW() {
    return UpdatedComponentsW(T{});
  }

  template <typename T, typename Ent>
  UpdatedComponents<const std::vector<T>*, Ent> UpdatedComponentsR() {
    return UpdatedComponentsR(T{});
  }

  template <typename T, typename Ent>
  RemovedComponentsHolder<T, Ent> RemovedComponents() {
    return RemovedComponents(T{});
  }

  template <typename T, typename Ent>
  std::vector<Ent>* Entities() {
    return Entities(T{});
  }
};

using MockEntity_t = Entity<EntityManagerMock>;
}  // namespace ecs
