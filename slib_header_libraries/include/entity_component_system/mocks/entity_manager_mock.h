#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "entity.h"

namespace ecs {
#define REGISTER_COMPONENT_TYPE(type)                                       \
  MOCK_METHOD(const type*, ComponentR, (type), (const));                    \
  MOCK_METHOD(const type*, ComponentR,                                      \
              (type, ecs::Entity<ecs::EntityManagerMock>&, std::uint64_t),  \
              (const));                                                     \
  MOCK_METHOD(type*, ComponentW, (type));                                   \
  MOCK_METHOD(type*, ComponentW,                                            \
              (type, ecs::Entity<ecs::EntityManagerMock>&, std::uint64_t)); \
  MOCK_METHOD(type*, AddComponent, (type));                                 \
  MOCK_METHOD(type*, AddComponent,                                          \
              (type, ecs::Entity<ecs::EntityManagerMock>&));                \
  MOCK_METHOD(void, RemoveComponent, (type));                               \
  MOCK_METHOD(void, RemoveComponent,                                        \
              (type, ecs::Entity<ecs::EntityManagerMock>&, std::uint64_t));

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
};
}  // namespace ecs
