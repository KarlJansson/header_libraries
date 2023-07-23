#include "json_to_table.hpp"

std::string json_input{
    "\"TableTest\": [\n{\n\"col1\": 1,\n\"col2\": \"name\",\n\"col3\": "
    "{\"test\": 1}\n},\n{\n\"col1\": "
    "2,\n\"col2\": \"name2\",\n\"col3\": {\"test2\": 2, \"test3\": {\"test4\": 4}}\n}\n]"};

TEST(StringManipulation, json_to_table) {
  auto json_to_table = json_util::JsonToTable{};
  json_to_table.Tablefy(json_input);
}

TEST(StringManipulation, json_to_table_concat) {
  auto json_to_table = json_util::JsonToTable{};
  auto concat = json_input + ",\n" + json_input;
  json_to_table.Tablefy(concat);
}
