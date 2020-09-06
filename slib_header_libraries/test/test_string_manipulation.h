#include <iostream>

#include "columnize.hpp"
#include "splitter.hpp"

TEST(StringManipulation, columnize_single_add) {
  sml::Columnize<4> table;
  table.Add("Value1"), table.Add("Value2"), table.Add("Value3"),
      table.Add("Value4");

  table.Add("Value5"), table.Add("Value6"), table.Add("Value7"),
      table.Add("Value8");

  std::string expectation =
      "Value1   Value2   Value3   Value4\n"
      "Value5   Value6   Value7   Value8\n";
  std::string actual = table.Get();
  EXPECT_EQ(expectation, actual);

  if (expectation != actual) {
    std::cout << "Expected:" << std::endl;
    std::cout << expectation;
    std::cout << "Actual:" << std::endl;
    std::cout << actual;
  }
}

TEST(StringManipulation, columnize_multi_add) {
  sml::Columnize<4> table;
  table.Add("Value1", "Value2", "Value3", "Value4");
  table.Add("Value5", "Value6", "Value7", "Value8");

  std::string expectation =
      "Value1   Value2   Value3   Value4\n"
      "Value5   Value6   Value7   Value8\n";
  std::string actual = table.Get();
  EXPECT_EQ(expectation, actual);

  if (expectation != actual) {
    std::cout << "Expected:" << std::endl;
    std::cout << expectation;
    std::cout << "Actual:" << std::endl;
    std::cout << actual;
  }
}

TEST(StringManipulation, splitter) {
  auto splitter = sml::Splitter<' '>();
  std::string splitter_string = "this string is splitting on white space";
  EXPECT_EQ(splitter.front(splitter_string), std::string("this"));
  EXPECT_EQ(splitter_string, std::string("string is splitting on white space"));
  EXPECT_EQ(splitter.back(splitter_string), std::string("space"));
  EXPECT_EQ(splitter_string, std::string("string is splitting on white"));
  std::vector<std::string> expecte_split = splitter.split(splitter_string);
  EXPECT_EQ(expecte_split, std::vector<std::string>(
                               {"string", "is", "splitting", "on", "white"}));
  EXPECT_EQ(splitter_string, std::string("string is splitting on white"));
  for (size_t i = 0; i < 5; ++i)
    EXPECT_NE(splitter.front(splitter_string), std::string(""));
  EXPECT_TRUE(splitter_string.empty());
  EXPECT_EQ(splitter.front(splitter_string), std::string(""));
  EXPECT_TRUE(splitter_string.empty());
  EXPECT_EQ(splitter.back(splitter_string), std::string(""));
  EXPECT_TRUE(splitter_string.empty());
  EXPECT_EQ(splitter.split(splitter_string), std::vector<std::string>({}));
  EXPECT_TRUE(splitter_string.empty());
}
