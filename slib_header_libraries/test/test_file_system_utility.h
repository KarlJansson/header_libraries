#include "file_system_utility.hpp"

TEST(FileSystemUtility, read_file) {
  fsu::FileReader file_reader;
  auto file = file_reader.FileToString("/etc/resolv.conf");
  EXPECT_NE(0, file.size());

  auto file_vec = file_reader.FileToStringVector("/etc/resolv.conf");
  EXPECT_NE(0, file_vec.size());
}
