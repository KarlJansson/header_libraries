#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "entity.h"

#define Type(t) std::type_index(typeid(t))

namespace ecs {
class EntityManagerMock {
 public:
  MOCK_METHOD(void, SyncSwap, ());
  MOCK_METHOD(Entity, CreateEntity, ());
  MOCK_METHOD(std::any&, AddComponent, (std::type_index));
  MOCK_METHOD(std::any&, AddComponent, (std::type_index, const Entity&));
  MOCK_METHOD(std::any&, ComponentR, (std::type_index));
  MOCK_METHOD(std::any&, ComponentW, (std::type_index));
  MOCK_METHOD(std::any&, ComponentR, (std::type_index, const Entity&, size_t));
  MOCK_METHOD(std::any&, ComponentW, (std::type_index, const Entity&, size_t));
  MOCK_METHOD(std::any&, ComponentsR, (std::type_index));
  MOCK_METHOD(std::any&, ComponentsW, (std::type_index));
  MOCK_METHOD(std::any&, ComponentsR, (std::type_index, const Entity&));
  MOCK_METHOD(std::any&, ComponentsW, (std::type_index, const Entity&));
  MOCK_METHOD(void, RemoveComponent, (std::type_index));
  MOCK_METHOD(void, RemoveComponent, (std::type_index, const Entity&, size_t));

  MOCK_METHOD(std::any&, AddedComponentsR, (std::type_index));
  MOCK_METHOD(std::any&, AddedComponentsW, (std::type_index));
  MOCK_METHOD(std::any&, UpdatedComponentsR, (std::type_index));
  MOCK_METHOD(std::any&, UpdatedComponentsW, (std::type_index));
  MOCK_METHOD(std::any&, RemovedComponents, (std::type_index));
  MOCK_METHOD(std::any&, Entities, (std::type_index));

  MOCK_METHOD(std::any&, ComponentCount, (std::type_index, const Entity&));
};
}  // namespace ecs

namespace ecss {
class EntityManagerMock {
 public:
  MOCK_METHOD(void, SyncSwap, ());
  MOCK_METHOD(Entity, CreateEntity, ());
  MOCK_METHOD(std::any&, AddComponent, (std::type_index));
  MOCK_METHOD(std::any&, AddComponent, (std::type_index, const Entity&));
  MOCK_METHOD(std::any&, Component, (std::type_index));
  MOCK_METHOD(std::any&, ComponentR, (std::type_index));
  MOCK_METHOD(std::any&, ComponentW, (std::type_index));
  MOCK_METHOD(std::any&, Component, (std::type_index, const Entity&, size_t));
  MOCK_METHOD(std::any&, ComponentR, (std::type_index, const Entity&, size_t));
  MOCK_METHOD(std::any&, ComponentW, (std::type_index, const Entity&, size_t));
  MOCK_METHOD(std::any&, Components, (std::type_index));
  MOCK_METHOD(std::any&, ComponentsR, (std::type_index));
  MOCK_METHOD(std::any&, ComponentsW, (std::type_index));
  MOCK_METHOD(std::any&, Components, (std::type_index, const Entity&));
  MOCK_METHOD(std::any&, ComponentsR, (std::type_index, const Entity&));
  MOCK_METHOD(std::any&, ComponentsW, (std::type_index, const Entity&));
  MOCK_METHOD(void, RemoveComponent, (std::type_index));
  MOCK_METHOD(void, RemoveComponent, (std::type_index, const Entity&, size_t));
};
}  // namespace ecss
