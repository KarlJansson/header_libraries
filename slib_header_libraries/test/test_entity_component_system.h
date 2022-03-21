#include "entity_manager.h"
#include "entity_manager_mock.h"
#include "system_manager.h"
#include "system_manager_mock.h"

using namespace ::testing;
using namespace ecs;

TEST(EntityManager, create_entity) {
  EntityManager_t ent_mgr;

  SystemManager<EntityManager_t> sys_mgr;
  sys_mgr.AddSystem<TestSystem>();
  sys_mgr.SyncSystems();
  sys_mgr.Step(ent_mgr);
}

TEST(EntityManagerMock, create_entity_and_components) {
  StrictMock<EntityManagerMock> ent_mgr;

  Entity ent(0);
  EXPECT_CALL(ent_mgr, CreateEntity()).WillOnce(Return(ent));

  int int_obj{1};
  EXPECT_CALL(ent_mgr, AddComponent(Matcher<int>(_), ent))
      .WillOnce(ReturnRef(int_obj));
  EXPECT_CALL(ent_mgr, ComponentW(Matcher<int>(_), ent, 0))
      .WillRepeatedly(Return(&int_obj));
  EXPECT_CALL(ent_mgr, ComponentR(Matcher<int>(_), ent, 0))
      .WillRepeatedly(Return(&int_obj));
  EXPECT_CALL(ent_mgr, RemoveComponent(Matcher<int>(_), ent, 0));

  std::vector<int> comps{0};
  std::vector<size_t> inds{0};
  std::vector<Entity_t> ents{ent};
  EXPECT_CALL(ent_mgr, AddedComponentsW(Matcher<int>(_)))
      .WillOnce(Return(UpdatedComponents(&comps, &ents, &inds)));
  EXPECT_CALL(ent_mgr, UpdatedComponentsW(Matcher<int>(_)))
      .WillOnce(Return(UpdatedComponents(&comps, &ents, &inds)));

  const std::vector<int> const_comps{0};
  EXPECT_CALL(ent_mgr, AddedComponentsR(Matcher<int>(_)))
      .WillOnce(Return(UpdatedComponents(&const_comps, &ents, &inds)));
  EXPECT_CALL(ent_mgr, UpdatedComponentsR(Matcher<int>(_)))
      .WillOnce(Return(UpdatedComponents(&const_comps, &ents, &inds)));

  EXPECT_CALL(ent_mgr, RemovedComponents(Matcher<int>(_)))
      .WillOnce(Return(RemovedComponentsHolder(&comps, &ents, &inds)));
  EXPECT_CALL(ent_mgr, Entities(Matcher<int>(_)))
      .WillOnce(Return(EntityHolder(&ents)));

  double double_obj{1};
  EXPECT_CALL(ent_mgr, AddComponent(Matcher<double>(_), ent))
      .WillOnce(ReturnRef(double_obj));
  EXPECT_CALL(ent_mgr, ComponentR(Matcher<double>(_), ent, 0))
      .WillRepeatedly(Return(&double_obj));
  EXPECT_CALL(ent_mgr, RemoveComponent(Matcher<double>(_), ent, 0));

  StrictMock<SystemManagerMock<Entity_t, EntityManagerMock>> sys_mgr;
  EXPECT_CALL(sys_mgr, AddSystem(Matcher<TestSystem>(_)));
  EXPECT_CALL(sys_mgr, RemoveSystem(Matcher<TestSystem>(_)));

  TestSystem system;
  system.Step(ent_mgr, sys_mgr);

  EXPECT_EQ(comps[0], 2);
  EXPECT_EQ(int_obj, 4);
}
