#pragma once

#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

namespace tu {
class TraceInstance {
 public:
  TraceInstance() {
    AddTraceLevel(0, "[Error]");
    AddTraceLevel(1, "[Warning]");
    AddTraceLevel(2, "[Trace]");
    AddTraceLevel(3, "[Debug4]");
    AddTraceLevel(4, "[Debug3]");
    AddTraceLevel(5, "[Debug2]");
    AddTraceLevel(6, "[Debug1]");
    AddTraceLevel(7, "[Debug]");
  }

  void AddTraceLevel(size_t level, const std::string& name) {
    trace_levels_[name] = level;
  }

  void SetTraceLevel(size_t level) { level_ = level; }
  size_t TraceLevel() { return level_; }

  void SetSquelch(bool squelch) { squelched_ = squelch; }
  void ToggleSquelch() { squelched_ = !squelched_; }
  bool Squelched() {
    if (disabled_) return true;
    return squelched_;
  }

  void ToggleDisabled() { disabled_ = !disabled_; }

  std::optional<size_t> TraceLevel(const std::string& str) {
    if (auto it = trace_levels_.find(str); it != std::end(trace_levels_))
      return it->second;
    return {};
  }

  template <typename T>
  void PreCheck(const T& x) {
    stream_ << x;
    auto str = stream_.str();
    while (!str.empty() && str.back() == ' ') str.pop_back();
    while (!str.empty() && str[0] == ' ') str = str.substr(1);
    auto l = TraceLevel(str);
    if (l.has_value() && l.value() > TraceLevel()) SetSquelch(true);
  }

  template <typename T>
  void PostCheck(const T& x) {
    auto str = stream_.str();
    if (str == "\n") SetSquelch(false);
    stream_ = std::stringstream();
  }

 private:
  std::stringstream stream_;

  bool disabled_{false};
  bool squelched_{false};
  size_t level_{std::numeric_limits<size_t>::max()};
  std::unordered_map<std::string, size_t> trace_levels_;
};
}  // namespace tu

template <typename T>
tu::TraceInstance& operator<<(tu::TraceInstance& ti, const T& x) {
  ti.PreCheck(x);
  if (!ti.Squelched()) std::cout << x;
  ti.PostCheck(x);
  return ti;
}
