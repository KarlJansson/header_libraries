#include "entity_manager.h"
#include "entity_manager_mock.h"
#include "system_manager.h"
#include "system_manager_mock.h"

using namespace ::testing;

TEST(EntityManager, create_entity) {
  ecs::EntityManager ent_mgr;

  ecs::SystemManager<ecs::EntityManager, ecs::Entity<ecs::EntityManager>>
      sys_mgr;
  sys_mgr.AddSystem<TestSystem>();
  sys_mgr.SyncSystems();
  sys_mgr.Step(ent_mgr);
}

TEST(EntityManagerMock, create_entity_and_components) {
  StrictMock<ecs::EntityManagerMock> ent_mgr;

  ecs::Entity<ecs::EntityManagerMock> ent(&ent_mgr);
  EXPECT_CALL(ent_mgr, CreateEntity()).WillOnce(Return(ent));

  int int_obj{1};
  EXPECT_CALL(ent_mgr, AddComponent(Matcher<int>(_), ent))
      .WillOnce(Return(&int_obj));
  EXPECT_CALL(ent_mgr, ComponentW(Matcher<int>(_), ent, 0))
      .WillRepeatedly(Return(&int_obj));
  EXPECT_CALL(ent_mgr, ComponentR(Matcher<int>(_), ent, 0))
      .WillRepeatedly(Return(&int_obj));
  EXPECT_CALL(ent_mgr, RemoveComponent(Matcher<int>(_), ent, 0));

  std::vector<int> comps{0};
  std::vector<size_t> inds{0};
  std::vector<ecs::Entity<ecs::EntityManagerMock>> ents{ent};
  EXPECT_CALL(ent_mgr, AddedComponentsW(Matcher<int>(_)))
      .WillOnce(Return(ecs::UpdatedComponents(&comps, &ents, &inds)));
  EXPECT_CALL(ent_mgr, UpdatedComponentsW(Matcher<int>(_)))
      .WillOnce(Return(ecs::UpdatedComponents(&comps, &ents, &inds)));

  const std::vector<int> const_comps{0};
  EXPECT_CALL(ent_mgr, AddedComponentsR(Matcher<int>(_)))
      .WillOnce(Return(ecs::UpdatedComponents(&const_comps, &ents, &inds)));
  EXPECT_CALL(ent_mgr, UpdatedComponentsR(Matcher<int>(_)))
      .WillOnce(Return(ecs::UpdatedComponents(&const_comps, &ents, &inds)));

  EXPECT_CALL(ent_mgr, RemovedComponents(Matcher<int>(_)))
      .WillOnce(Return(ecs::RemovedComponentsHolder(&comps, &ents, &inds)));
  EXPECT_CALL(ent_mgr, Entities(Matcher<int>(_))).WillOnce(Return(&ents));

  double double_obj{1};
  EXPECT_CALL(ent_mgr, AddComponent(Matcher<double>(_), ent))
      .WillOnce(Return(&double_obj));
  EXPECT_CALL(ent_mgr, ComponentR(Matcher<double>(_), ent, 0))
      .WillRepeatedly(Return(&double_obj));
  EXPECT_CALL(ent_mgr, RemoveComponent(Matcher<double>(_), ent, 0));

  StrictMock<ecs::SystemManagerMock<ecs::Entity<ecs::EntityManagerMock>,
                                    ecs::EntityManagerMock>>
      sys_mgr;
  EXPECT_CALL(sys_mgr, AddSystem(Matcher<TestSystem>(_)));
  EXPECT_CALL(sys_mgr, RemoveSystem(Matcher<TestSystem>(_)));

  TestSystem system;
  system.Step<ecs::Entity<ecs::EntityManagerMock>>(ent_mgr, sys_mgr);

  EXPECT_EQ(comps[0], 2);
  EXPECT_EQ(int_obj, 4);
}
