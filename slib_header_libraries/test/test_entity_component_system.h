#define UNIT_TEST

#include "entity_manager.h"
#include "entity_manager_mock.h"
#include "system_manager.h"
#include "system_manager_mock.h"

using namespace ::testing;
using namespace ecs;

class TestSystem {
 public:
  void Step(ecs::EntityManager& ent_mgr, SystemManager_t& sys_mgr) {
    auto ent = ent_mgr.CreateEntity();

    {
      auto& comp = ent_mgr.AddComponent<int>(ent);
      comp += 1;
    }

    if (auto comp = ent_mgr.ComponentW<int>(ent); comp) *comp += 1;

    auto comp = ent_mgr.AddComponent<double>(ent);
    auto comp_2 = ent_mgr.ComponentR<double>(ent);

    for (auto& c : ent_mgr.ComponentsR<int>(ent)) {
    }
    for (auto& c : ent_mgr.ComponentsW<int>(ent)) c += 1;

    ent_mgr.RemoveComponent<int>(ent);
    ent_mgr.RemoveComponent<double>(ent);

    for (auto& c : ent_mgr.ComponentsW<int>(ent)) c += 1;
    for (auto [c, ent] : ent_mgr.AddedComponentsW<int>()) c += 1;
    for (auto [c, ent] : ent_mgr.AddedComponentsR<int>()) {
    }

    for (auto [c, ent] : ent_mgr.UpdatedComponentsW<int>()) c += 1;
    for (auto [c, ent] : ent_mgr.UpdatedComponentsR<int>()) {
    }

    for (auto [c, ent] : ent_mgr.RemovedComponents<int>()) {
    }

    for (auto& ent : ent_mgr.Entities<int>()) {
    }

    sys_mgr.RemoveSystem<TestSystem>();
    sys_mgr.AddSystem<TestSystem>();
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};

TEST(EntityManager, create_entity) {
  ecs::EntityManager ent_mgr;

  SystemManager_t sys_mgr;
  sys_mgr.AddSystem<TestSystem>();
  sys_mgr.SyncSystems();
  sys_mgr.Step(ent_mgr);
}

TEST(EntityManagerMock, create_entity_and_components) {
  StrictMock<EntityManagerMock> ent_mgr_mock;
  EntityManager ent_mgr(&ent_mgr_mock);

  Entity ent(0);
  EXPECT_CALL(ent_mgr_mock, CreateEntity()).WillOnce(Return(ent));

  std::any int_obj = int{1};
  EXPECT_CALL(ent_mgr_mock, AddComponent(Type(int), ent))
      .WillOnce(ReturnRef(int_obj));
  EXPECT_CALL(ent_mgr_mock, ComponentW(Type(int), ent, 0))
      .WillRepeatedly(ReturnRef(int_obj));
  EXPECT_CALL(ent_mgr_mock, ComponentR(Type(int), ent, 0))
      .WillRepeatedly(ReturnRef(int_obj));
  EXPECT_CALL(ent_mgr_mock, RemoveComponent(Type(int), ent, 0));

  std::vector<int> comps{0};
  std::vector<size_t> inds{0};
  std::vector<Entity_t> ents{ent};
  std::any components = UpdatedComponents(&comps, &ents, &inds);
  EXPECT_CALL(ent_mgr_mock, AddedComponentsW(Type(int)))
      .WillOnce(ReturnRef(components));
  EXPECT_CALL(ent_mgr_mock, UpdatedComponentsW(Type(int)))
      .WillOnce(ReturnRef(components));

  std::any ent_comps = EntityComponents<int>(0, [](auto) -> int* { return 0; });
  EXPECT_CALL(ent_mgr_mock, ComponentsW(Type(int), ent))
      .WillRepeatedly(ReturnRef(ent_comps));
  std::any const_ent_comps =
      EntityComponents<const int>(0, [](auto) -> int* { return 0; });
  EXPECT_CALL(ent_mgr_mock, ComponentsR(Type(int), ent))
      .WillRepeatedly(ReturnRef(const_ent_comps));

  const std::vector<int> const_comps{0};
  std::any const_components = UpdatedComponents(&const_comps, &ents, &inds);
  EXPECT_CALL(ent_mgr_mock, AddedComponentsR(Type(int)))
      .WillOnce(ReturnRef(const_components));
  EXPECT_CALL(ent_mgr_mock, UpdatedComponentsR(Type(int)))
      .WillOnce(ReturnRef(const_components));

  std::any removed_components = RemovedComponentsHolder(&comps, &ents, &inds);
  std::any entities = EntityHolder(&ents);
  EXPECT_CALL(ent_mgr_mock, RemovedComponents(Type(int)))
      .WillOnce(ReturnRef(removed_components));
  EXPECT_CALL(ent_mgr_mock, Entities(Type(int))).WillOnce(ReturnRef(entities));

  std::any double_obj = double{1};
  EXPECT_CALL(ent_mgr_mock, AddComponent(Type(double), ent))
      .WillOnce(ReturnRef(double_obj));
  EXPECT_CALL(ent_mgr_mock, ComponentR(Type(double), ent, 0))
      .WillRepeatedly(ReturnRef(double_obj));
  EXPECT_CALL(ent_mgr_mock, RemoveComponent(Type(double), ent, 0));

  StrictMock<SystemManagerMock> sys_mgr_mock;
  EXPECT_CALL(sys_mgr_mock, AddSystem(Type(TestSystem)));
  EXPECT_CALL(sys_mgr_mock, RemoveSystem(Type(TestSystem)));

  TestSystem system;
  SystemManager_t sys_mgr(&sys_mgr_mock);
  system.Step(ent_mgr, sys_mgr);

  EXPECT_EQ(std::any_cast<int>(comps[0]), 2);
  EXPECT_EQ(std::any_cast<int>(int_obj), 3);
}
