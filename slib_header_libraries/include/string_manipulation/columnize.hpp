#pragma once

#include <array>
#include <string>
#include <vector>

namespace sml {
template <std::uint32_t COLUMNS, std::uint32_t PADDING = 3>
class Columnize {
 public:
  template <typename T, typename... Args>
  void Add(T str, Args... args) {
    Add(str);
    Add(args...);
  }

  template <typename T>
  void Add(T value) {
    std::string val = value;
    auto& length = lengths_[values_.size() % COLUMNS];
    if (length < val.size()) length = val.size();
    values_.emplace_back(val);
  }

  std::string Get() {
    std::string out;
    for (size_t i = 0; i < values_.size(); ++i) {
      auto& value = values_[i];
      auto length = lengths_[i % COLUMNS];
      out += value;
      out += (i % COLUMNS == COLUMNS - 1)
                 ? "\n"
                 : std::string(PADDING + length - value.size(), ' ');
    }
    return out;
  }

 private:
  std::vector<std::string> values_;
  std::array<std::uint32_t, COLUMNS> lengths_{0};
};
}  // namespace sml
