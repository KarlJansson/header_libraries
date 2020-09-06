#pragma once

#include <string>
#include <vector>

namespace sml {
template <char SPLIT_CHAR = ','>
class Splitter {
 public:
  std::string front(std::string& str, char split_char = SPLIT_CHAR) {
    auto p = str.find_first_of(split_char);
    return split(str, 0, p);
  }

  std::string back(std::string& str, char split_char = SPLIT_CHAR) {
    auto p = str.find_last_of(split_char);
    return split(str, str.size(), p);
  }

  std::vector<std::string> split(std::string str,
                                 char split_char = SPLIT_CHAR) {
    std::string part;
    std::vector<std::string> out;
    do {
      part = front(str, split_char);
      if (!part.empty()) out.emplace_back(part);
    } while (!part.empty());
    return out;
  }

 private:
  std::string split(std::string& str, size_t p0, size_t p1) {
    std::string out;
    if (p0 != std::string::npos && p1 != std::string::npos) {
      if (p0 < p1) {
        out = str.substr(p0, p1);
        str = str.substr(p1 + 1);
      } else if (p0 > p1) {
        out = str.substr(p1 + 1, p0);
        str = str.substr(0, p1);
      }
    } else {
      out = str;
      str = "";
    }
    return out;
  }
};
}  // namespace sml
