#pragma once

// #include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <source_location>
#include <sstream>
#include <string>
#include <unordered_map>

namespace tu {
class TStr {
 public:
  template <typename... Args>
  TStr(const char* fmt, Args... args) {
    ss_ << std::hex << std::time(nullptr) << "\e" << fmt << "\e";
    (ss_ << ... << (ToString(args) + "\e"));
  }

  std::string str() { return ss_.str(); }
  std::string str_pretty() {
    std::string str;
    while (std::getline(ss_, str, '\e')) {
      std::cout << str << std::endl;
    }
    return ss_.str();
  }

 private:
  template <typename T>
  std::string ToString(T s) {
    return std::to_string(s);
  }

  std::stringstream ss_;
};

template <>
std::string TStr::ToString<std::string>(std::string s) {
  return s;
}

template <>
std::string TStr::ToString<const char*>(const char* s) {
  return std::string(s);
}

inline void Trace(TStr&& fmt, const std::source_location location =
                                  std::source_location::current()) {
  std::string_view file_path{location.file_name()};
  std::cout << file_path.substr(file_path.find_last_of('/') + 1) << "("
            << location.line() << ":" << location.column() << ")";
  std::cout << fmt.str_pretty() << std::endl;
  // std::cout << location.function_name() << std::endl;
}

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
