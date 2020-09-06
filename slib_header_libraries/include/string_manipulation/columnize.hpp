#pragma once

#include <array>
#include <iostream>
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
    if (values_.size() % COLUMNS == COLUMNS - 1) divider_.emplace_back("");
  }

  void AddDivider(const std::string& divisor) {
    divider_[values_.size() / COLUMNS] = divisor;
  }

  std::string Get() {
    std::string out;
    std::uint32_t divisor_length = COLUMNS * PADDING;
    for (auto l : lengths_) divisor_length += l;
    for (size_t i = 0; i < values_.size(); ++i) {
      std::uint32_t rest = i % COLUMNS;
      auto& value = values_[i];
      auto length = lengths_[rest];
      if (!divider_[i / COLUMNS].empty()) {
        auto& div = divider_[i / COLUMNS];
        for (int ii = 0; ii < divisor_length; ii += div.size()) out += div;
        out += "\n";
        div.clear();
      }

      out += value;
      out += (rest == COLUMNS - 1)
                 ? "\n"
                 : std::string(PADDING + length - value.size(), ' ');
    }
    for (auto& d : divider_) {
      if (!d.empty()) {
        for (int ii = 0; ii < divisor_length; ii += d.size()) out += d;
        out += "\n";
      }
    }
    return out;
  }

 private:
  std::vector<std::string> values_;
  std::vector<std::string> divider_{""};
  std::array<std::uint32_t, COLUMNS> lengths_{0};
};
}  // namespace sml
