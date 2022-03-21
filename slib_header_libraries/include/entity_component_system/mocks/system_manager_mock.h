#pragma once

#include "ecs_macros.h"

class TestSystem {
 public:
  system_step() {
    auto ent = ent_mgr.CreateEntity();

    {
      auto& comp = ent_add_component(ent, int);
      comp += 1;
    }

    if (auto comp = ent_component_w(ent, int); comp) *comp += 1;

    auto comp = ent_add_component(ent, double);
    auto comp_2 = ent_component_r(ent, double);

    for (auto& c : ent_components_r(ent, int)) {
    }
    for (auto& c : ent_components_w(ent, int)) c += 1;

    ent_remove_component(ent, int);
    ent_remove_component(ent, double);

    for (auto& c : ent_components_w(ent, int)) c += 1;
    for (const auto& [c, ent] : emgr_added_components_w(int)) c += 1;
    for (const auto& [c, ent] : emgr_added_components_r(int)) {
    }

    for (const auto& [c, ent] : emgr_updated_components_w(int)) c += 1;
    for (const auto& [c, ent] : emgr_updated_components_r(int)) {
    }

    for (const auto& [c, ent] : emgr_removed_components(int)) {
    }

    for (const auto& ent : emgr_entities(int)) {
    }

    smgr_remove_system(TestSystem);
    smgr_add_system(TestSystem);
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};

namespace ecs {
#define REGISTER_SYSTEM_TYPE(type)         \
  MOCK_METHOD(void, AddSystem, (type));    \
  MOCK_METHOD(void, RemoveSystem, (type)); \
  MOCK_METHOD(type*, System, (type));

template <typename Ent, typename EntMgr>
class SystemManagerMock {
 public:
  REGISTER_SYSTEM_TYPE(TestSystem);

  MOCK_METHOD(void, Step, (EntMgr&));
  MOCK_METHOD(void, SyncSystems, ());

  template <typename T>
  void AddSystem() {
    AddSystem(T{});
  }

  template <typename T>
  void RemoveSystem() {
    RemoveSystem(T{});
  }

  template <typename T>
  T* System() {
    return System(T{});
  }
};
}  // namespace ecs
