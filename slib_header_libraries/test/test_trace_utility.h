#include "trace_utility.hpp"

TEST(TraceUtility, format_tracing) {
  std::string test_str{"var_str"};
  tu::Trace("Hello test");
  tu::Trace(
      {"Test trace: int: {} float: {} string: {}", 5, 2.20f, "test", test_str});
  tu::Trace({"1", 5, 2.20f});
}

TEST(TraceUtility, trace_objects) {}
