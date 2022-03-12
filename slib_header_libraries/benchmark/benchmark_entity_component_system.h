#include "entity_manager.h"
#include "system_manager.h"

void BM_create_remove_entity(benchmark::State& state) {
  ecs::EntityManager ent_mgr;
  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
    ent_mgr.CreateEntity();
  }
}
BENCHMARK(BM_create_remove_entity)->DenseThreadRange(1, 1);

void BM_add_component(benchmark::State& state) {
  ecs::EntityManager ent_mgr;
  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
    ent_mgr.AddComponent<int, ecs::Entity<ecs::EntityManager>>();
    ent_mgr.SyncSwap();
  }
}
BENCHMARK(BM_add_component)->DenseThreadRange(1, 1);

void BM_get_component_r(benchmark::State& state) {
  ecs::EntityManager ent_mgr;
  ent_mgr.AddComponent<int, ecs::Entity<ecs::EntityManager>>();
  ent_mgr.SyncSwap();
  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
    benchmark::DoNotOptimize(
        ent_mgr.ComponentR<int, ecs::Entity<ecs::EntityManager>>());
  }
}
BENCHMARK(BM_get_component_r)->DenseThreadRange(1, 1);

void BM_get_component_w(benchmark::State& state) {
  ecs::EntityManager ent_mgr;
  ent_mgr.AddComponent<int, ecs::Entity<ecs::EntityManager>>();
  ent_mgr.SyncSwap();
  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
    benchmark::DoNotOptimize(
        ent_mgr.ComponentW<int, ecs::Entity<ecs::EntityManager>>());
  }
}
BENCHMARK(BM_get_component_w)->DenseThreadRange(1, 1);

void BM_add_remove_component(benchmark::State& state) {
  ecs::EntityManager ent_mgr;
  auto ent = ent_mgr.CreateEntity();

  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
    auto comp = ent.AddComponent<int, ecs::Entity<ecs::EntityManager>>();
    *comp = 1;
    ent_mgr.SyncSwap();
    comp = ent.ComponentW<int, ecs::Entity<ecs::EntityManager>>();
    *comp = 2;
    ent.RemoveComponent<int>();
    ent_mgr.SyncSwap();
  }
}
BENCHMARK(BM_add_remove_component)->DenseThreadRange(1, 1);

void BM_sync_swap(benchmark::State& state) {
  ecs::EntityManager ent_mgr;

  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
    ent_mgr.SyncSwap();
  }
}
BENCHMARK(BM_sync_swap)->DenseThreadRange(1, 1);
