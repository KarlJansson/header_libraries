#pragma once

#include <any>
#include <chrono>
#include <typeindex>
#include <unordered_map>

#include "chunk_list.hpp"
#include "tbb_templates.hpp"

namespace ecs {
template <typename EntMgr>
class SystemManager {
 public:
  void Step(EntMgr& ent_mgr) {
    for (auto& execution_group : execution_order_)
      tbb_templates::parallel_for(execution_group, [&](size_t i) {
        execution_group[i]->execute(ent_mgr);
      });
  }

  struct SystemHolder {
    std::any system;
    std::function<void(EntMgr&)> execute;
    std::function<std::vector<std::type_index>()> dependencies;
  };

  template <typename T>
  void AddSystem() {
    add_system_cache_.push_back([this]() {
      SystemHolder sys_holder;
      auto sys = std::make_shared<T>();
      std::weak_ptr<T> sys_weak = sys;
      sys_holder.system = std::any(sys);
      sys_holder.execute = [this, sys_weak](EntMgr& ent_mgr) {
        if (auto sys = sys_weak.lock(); sys) sys->Step(ent_mgr, *this);
      };
      sys_holder.dependencies = [sys_weak]() -> auto {
        if (auto sys = sys_weak.lock(); sys) return sys->Dependencies();
        return std::vector<std::type_index>{};
      };
      systems_.emplace(typeid(T), sys_holder);
      sys->Init();
    });
  }

  template <typename T>
  void RemoveSystem() {
    remove_system_cache_.push_back([this]() { systems_.erase(typeid(T)); });
  }

  template <typename T>
  T* System() {
    if (auto it = systems_.find(typeid(T)); it != std::end(systems_))
      return std::any_cast<std::shared_ptr<T>>(it->second.system).get();
    return nullptr;
  }

  void SyncSystems() {
    for (auto& sys : add_system_cache_) sys();
    for (auto& sys : remove_system_cache_) sys();

    if (!remove_system_cache_.empty() || !add_system_cache_.empty())
      CalculateExecutionOrder();

    remove_system_cache_.clear();
    add_system_cache_.clear();
  }

 private:
  void CalculateExecutionOrder() {
    std::unordered_map<std::type_index, size_t> group_ids;
    bool repeat = true;
    while (repeat) {
      repeat = false;
      for (const auto& [type_ind, sys] : systems_) {
        auto deps = sys.dependencies();
        auto& group_id = group_ids[type_ind];
        for (auto d : deps) {
          auto dep_it = systems_.find(d);
          if (dep_it != std::end(systems_)) {
            auto dep_group_id = group_ids[d];
            auto deps_deps = dep_it->second.dependencies();
            auto it = std::find(deps_deps.begin(), deps_deps.end(), type_ind);
            if (dep_group_id >= group_id && it == std::end(deps_deps)) {
              group_id = dep_group_id + 1;
              repeat = true;
            }
          }
        }
      }
    }

    execution_order_.clear();
    if (!group_ids.empty()) {
      for (const auto& [type_ind, prio] : group_ids) {
        while (prio >= execution_order_.size()) execution_order_.emplace_back();
        execution_order_[prio].emplace_back(&systems_[type_ind]);
      }
    }
  }

  std::vector<std::vector<SystemHolder*>> execution_order_;
  std::unordered_map<std::type_index, SystemHolder> systems_;

  tbb::concurrent_vector<std::function<void(void)>> add_system_cache_;
  tbb::concurrent_vector<std::function<void(void)>> remove_system_cache_;
};

}  // namespace ecs
