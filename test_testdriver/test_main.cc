#ifndef TEST_TARGET
#define TEST_TARGET
#endif

#include "precomp.h"

#include "test_entity_component_system.h"
#include "test_file_system_utility.h"
#include "test_string_manipulation.h"
#include "test_entity_manager_simple.h"

int main(int argc, char** args) {
  ::testing::InitGoogleTest(&argc, args);
  return RUN_ALL_TESTS();
}