#include "entity_manager_simple.h"
#include "tbb_templates.hpp"

TEST(EntityManagerSimple, create_entity) {
  ecss::EntityManager_t ent_mgr;
  ent_mgr.RegisterType<size_t>();
  ent_mgr.RegisterType<std::uint32_t>();

  auto ent = ent_mgr.CreateEntity();
  auto& ent_comp = ent_mgr.AddComponent<size_t>(ent);
  ent_comp = 2;

  auto comp = ent_mgr.Component<size_t>(ent);
  EXPECT_EQ(ent_comp, *comp);

  auto& ent_comp_2 = ent_mgr.AddComponent<size_t>(ent);
  ent_comp_2 = 5;

  auto ent_2 = ent_mgr.CreateEntity();
  auto& ent_comp_3 = ent_mgr.AddComponent<size_t>(ent_2);
  ent_comp_3 = 55;

  auto& exit_code = ent_mgr.AddComponent<std::uint32_t>();
  exit_code = 1;

  auto exit_comp = ent_mgr.Component<std::uint32_t>();
  EXPECT_EQ(exit_code, *exit_comp);

  size_t res{0};
  for (auto& [comp, ent] : ent_mgr.Components<size_t>()) res += comp;
  EXPECT_EQ(res, ent_comp + ent_comp_2 + ent_comp_3);

  res = 0;
  for (auto& comp : ent_mgr.Components<size_t>(ent)) res += comp;
  EXPECT_EQ(res, ent_comp + ent_comp_2);
}

TEST(EntityManagerSimple, parallel_test) {
  ecss::EntityManager_t ent_mgr;
  ent_mgr.RegisterType<size_t>();

  tbb::parallel_for(0, 1200, [&](auto i) {
    auto ent = ent_mgr.CreateEntity();
    auto& comp = ent_mgr.AddComponent<size_t>(ent);
    comp = i;
  });

  auto& comps = ent_mgr.Components<size_t>();
  for (auto& [comp, ent] : comps) {
    auto ent_comp = ent_mgr.Component<size_t>(ent);
    EXPECT_EQ(comp, *ent_comp);
  }
  tbb_templates::parallel_for(comps, [&](auto i) {
    auto& [comp, ent] = comps[i];
    auto ent_comp = ent_mgr.Component<size_t>(ent);
    EXPECT_EQ(comp, *ent_comp);
    for (auto& c : ent_mgr.Components<size_t>(ent)) EXPECT_EQ(comp, c);
  });
}
