#include "entity_manager.h"
#include "entity_manager_mock.h"
#include "system_manager.h"
#include "system_manager_mock.h"

using namespace ::testing;

class TestSystem {
 public:
  template <typename EntMgr, typename Ent, typename SysMgr>
  void Step(EntMgr& ent_mgr, SysMgr& sys_mgr) {
    auto ent = ent_mgr.CreateEntity();

    if (auto comp = ent.template AddComponent<int, Ent>(); comp) *comp += 1;
    if (auto comp = ent.template ComponentW<int, Ent>(); comp) *comp += 1;

    auto comp = ent.template AddComponent<double, Ent>();
    auto comp_2 = ent.template ComponentR<double, Ent>();

    for (auto c : ent.template ComponentsR<int, Ent>()) {
    }
    for (auto c : ent.template ComponentsW<int, Ent>()) *c += 1;

    ent.template RemoveComponent<int>();
    ent.template RemoveComponent<double>();

    for (auto c : ent.template ComponentsW<int, Ent>()) *c += 1;
  }

  void Init() {}
  std::vector<std::type_index> Dependencies() { return {}; }
};

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

  double double_obj{1};
  EXPECT_CALL(ent_mgr, AddComponent(Matcher<double>(_), ent))
      .WillOnce(Return(&double_obj));
  EXPECT_CALL(ent_mgr, ComponentR(Matcher<double>(_), ent, 0))
      .WillRepeatedly(Return(&double_obj));
  EXPECT_CALL(ent_mgr, RemoveComponent(Matcher<double>(_), ent, 0));

  ecs::SystemManager<ecs::EntityManagerMock,
                     ecs::Entity<ecs::EntityManagerMock>>
      sys_mgr;
  sys_mgr.AddSystem<TestSystem>();
  sys_mgr.SyncSystems();
  sys_mgr.Step(ent_mgr);

  EXPECT_EQ(int_obj, 4);
}
