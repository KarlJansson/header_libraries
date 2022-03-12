#pragma once

#include <any>
#include <chrono>
#include <typeindex>
#include <unordered_map>

#include "chunk_list.hpp"
#include "tbb_templates.hpp"

namespace ecs {
template <typename EntMgr, typename Ent>
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
    add_system_cache_.push_front([this]() {
      SystemHolder sys_holder;
      auto sys = std::make_shared<T>();
      std::weak_ptr<T> sys_weak = sys;
      sys_holder.system = std::any(sys);
      sys_holder.execute = [this, sys_weak](EntMgr& ent_mgr) {
        if (auto sys = sys_weak.lock(); sys)
          sys->template Step<Ent, EntMgr, SystemManager>(ent_mgr, *this);
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
    remove_system_cache_.push_front([this]() { systems_.erase(typeid(T)); });
  }

  template <typename T>
  T* System() {
    if (auto it = systems_.find(typeid(T)); it != std::end(systems_))
      return std::any_cast<std::shared_ptr<T>>(it->second.system).get();
    return nullptr;
  }

  void SyncSystems() {
    bool update_execution_order = false;
    while (auto sys = add_system_cache_.back()) {
      (*sys)();
      add_system_cache_.pop_back();
      update_execution_order = true;
    }
    while (auto sys = remove_system_cache_.back()) {
      (*sys)();
      remove_system_cache_.pop_back();
      update_execution_order = true;
    }
    if (update_execution_order) CalculateExecutionOrder();
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

  synchronization::ChunkList<std::function<void(void)>, 16> add_system_cache_;
  synchronization::ChunkList<std::function<void(void)>, 16>
      remove_system_cache_;
};
}  // namespace ecs
