#pragma once

#include <regex>
#include <string>

namespace sml {
template <char splitter = ','>
class Regexer {
 public:
  void replace(std::string& source, const std::string& find,
               const std::string& replace) {
    source = std::regex_replace(source, std::regex("\\" + find), replace);
  }

  bool contains(std::string& source, const std::string& find) {
    std::regex regex(
        find, std::regex_constants::ECMAScript | std::regex_constants::icase);
    return std::regex_search(source, regex);
  }
};
}  // namespace sml
