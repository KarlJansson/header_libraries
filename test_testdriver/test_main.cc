#ifndef TEST_TARGET
#define TEST_TARGET
#endif

#include "precomp.h"

#include "test_string_manipulation.h"

int main(int argc, char** args) {
  ::testing::InitGoogleTest(&argc, args);
  return RUN_ALL_TESTS();
}