#pragma once

#include "entity.h"

class TestSystem {
 public:
  template <typename Ent, typename EntMgr, typename SysMgr>
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr) {
    auto ent = ent_mgr.CreateEntity();

    if (auto comp = ent_add_component(ent, int); comp) *comp += 1;
    if (auto comp = ent_component_w(ent, int); comp) *comp += 1;

    auto comp = ent_add_component(ent, double);
    auto comp_2 = ent_component_r(ent, double);

    for (auto c : ent_components_r(ent, int)) {
    }
    for (auto c : ent_components_w(ent, int)) *c += 1;

    ent_remove_component(ent, int);
    ent_remove_component(ent, double);

    for (auto c : ent_components_w(ent, int)) *c += 1;
    for (auto [c, ent] : emgr_added_components_w(ent_mgr, int)) *c += 1;
    for (auto [c, ent] : emgr_added_components_r(ent_mgr, int)) {
    }

    for (auto [c, ent] : emgr_updated_components_w(ent_mgr, int)) *c += 1;
    for (auto [c, ent] : emgr_updated_components_r(ent_mgr, int)) {
    }

    for (auto [c, ent] : emgr_removed_components(ent_mgr, int)) {
    }

    if (auto ents = emgr_entities(ent_mgr, int); ents) {
      for (auto ent : *ents) {
      }
    }

    smgr_remove_system(sys_mgr, TestSystem);
    smgr_add_system(sys_mgr, TestSystem);
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