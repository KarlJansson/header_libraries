#pragma once

#include <functional>
#include <limits>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#define ent_add_component(t, e) ent_mgr.template AddComponent<t>(e)
#define ent_component_w(t, e) ent_mgr.template ComponentW<t>(e)
#define ent_component_r(t, e) ent_mgr.template ComponentR<t>(e)
#define ent_components_r(t, e) ent_mgr.template ComponentsR<t>(e)
#define ent_components_w(t, e) ent_mgr.template ComponentsW<t>(e)
#define ent_remove_component(t, e) ent_mgr.template RemoveComponent<t>(e)

#define emgr_add_component(t) ent_mgr.template AddComponent<t>()
#define emgr_remove_component(t) ent_mgr.template RemoveComponent<t>()
#define emgr_component_w(t) ent_mgr.template ComponentW<t>()
#define emgr_component_r(t) ent_mgr.template ComponentR<t>()
#define emgr_components_w(t) ent_mgr.template ComponentsW<t>()
#define emgr_components_r(t) ent_mgr.template ComponentsR<t>()
#define emgr_added_components_w(t) ent_mgr.template AddedComponentsW<t>()
#define emgr_added_components_r(t) ent_mgr.template AddedComponentsR<t>()
#define emgr_updated_components_w(t) ent_mgr.template UpdatedComponentsW<t>()
#define emgr_updated_components_r(t) ent_mgr.template UpdatedComponentsR<t>()
#define emgr_removed_components(t) ent_mgr.template RemovedComponents<t>()
#define emgr_entities(t) ent_mgr.template Entities<t>()

#define smgr_step_systems(emgr) \
  sys_mgr.template Step(emgr);  \
  emgr.template SyncSwap();     \
  sys_mgr.template SyncSystems()
#define smgr_add_system(t) sys_mgr.template AddSystem<t>()
#define smgr_remove_system(t) sys_mgr.template RemoveSystem<t>()

#define system_step()                         \
  template <typename EntMgr, typename SysMgr> \
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr)

#define system_step_default()                                \
  void Init() {}                                             \
  std::vector<std::type_index> Dependencies() { return {}; } \
  template <typename EntMgr, typename SysMgr>                \
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr)
