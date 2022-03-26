#pragma once

namespace ecs {
class SystemManagerMock {
 public:
  MOCK_METHOD(void, Step, (class EntityManager&));
  MOCK_METHOD(void, SyncSystems, ());
  MOCK_METHOD(void, AddSystem, (const std::type_index));
  MOCK_METHOD(void, RemoveSystem, (const std::type_index));
  MOCK_METHOD(std::any&, System, (const std::type_index));
};
}  // namespace ecs
