#pragma once

#include <sstream>
#include <string>
#include <string_view>

#include "columnize.hpp"

namespace json_util {
class JsonToTable final {
 public:
  std::string Tablefy(std::string& json_input) {
    auto json_it = json_input.begin();

    auto advance = [&](auto& it, std::set<char>&& ends, auto func) {
      while (it != json_input.end() && ends.find(*it) == ends.end()) {
        func(*it, ends);
        if (it == json_input.end()) break;
        it++;
      }
    };
    auto parse_value = [&](auto& it, auto& val) {
      val.emplace_back();
      ParseValueToString(it, json_input.end(), val.back());
    };
    auto parse_name = [&](auto& it, auto& name, auto skip) {
      if (it == json_input.end()) return;
      it++;

      if (!skip) {
        name.emplace_back();
        advance(it, {':'}, [&](auto c, auto) {
          if (c != '"' && c != ' ' && c != '\n') name.back() += c;
        });
      } else
        advance(it, {':'}, [&](auto, auto) {});
    };
    auto parse_column = [&](auto& it, auto& names, auto& values, auto skip) {
      parse_name(it, names, skip);
      parse_value(it, values);
    };
    auto parse_row = [&](auto& it, auto& names, auto& values) {
      auto skip = !names.empty();
      while (it != json_input.end() && *it != '}')
        parse_column(it, names, values, skip);
    };
    auto print_table = [](auto& names, auto& values) {
      sml::ColumnizeDynamic t(names.size());
      t.AddDivider("-");
      for (auto& n : names) t.Add(n);
      t.AddDivider("-");
      for (auto& v : values) t.Add(v);
      t.AddDivider("-");
      std::cout << t.Get();
    };
    auto parse_table = [&](auto& it) {
      std::vector<std::string> names;
      std::vector<std::string> values;
      advance(it, {'{'}, [](auto, auto) {});
      while (it != json_input.end() && *it != ']') {
        parse_row(it, names, values);
        advance(it, {'{', ']'}, [](auto, auto) {});
      }
      print_table(names, values);
    };

    while (json_it != json_input.end()) {
      if (*json_it == '[') {
        std::cout << *json_it << std::endl;
        parse_table(json_it);
      } else {
        std::cout << *json_it;
        json_it++;
      }
    }

    std::cout << "\nInput:\n" << json_input << std::endl;

    return {};
  }

 private:
  void ParseValueToString(std::string::iterator& it,
                          const std::string::iterator& end_it, std::string& out,
                          const std::string indent = "") {
    if (it == end_it) return;
    it++;

    while (it != end_it) {
      if (*it == '{')
        ParseValueToString(it, end_it, out, indent + "  ");
      else if (*it == '}')
        return;

      if (it == end_it) break;
      if (indent.empty() and (*it == ',' or *it == '}')) return;
      if (*it != ' ' and *it != '\n' and *it != '"') out += *it;
      it++;
    }
  }
};
}  // namespace json_util
