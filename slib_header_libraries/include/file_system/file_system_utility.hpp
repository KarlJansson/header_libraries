#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fsu {
class FileReader {
 public:
  std::string FileToString(const std::string& path) {
    std::string result;
    if (std::filesystem::exists(path)) {
      std::ifstream input(path,
                          std::ios::ate | std::ios::binary | std::ios::in);
      if (input.is_open()) {
        result.assign(input.tellg(), ' ');
        input.seekg(0, std::ios_base::beg);
        input.read(&result[0], result.size());
      } else {
        std::cerr << "FileReader could not open file" << std::endl;
      }
    } else {
      std::cerr << "File " << path << " does not exist" << std::endl;
    }
    return result;
  }

  std::vector<std::string> FileToStringVector(const std::string& path) {
    std::vector<std::string> result;
    if (std::filesystem::exists(path)) {
      std::ifstream input(path, std::ios::binary | std::ios::in);
      if (input.is_open()) {
        std::string line;
        while (std::getline(input, line)) result.emplace_back(line);
      } else {
        std::cerr << "FileReader could not open file" << std::endl;
      }
    } else {
      std::cerr << "File " << path << " does not exist" << std::endl;
    }
    return result;
  }

  std::vector<std::vector<size_t>> FileToNumberMatrix(const std::string& path) {
    std::vector<std::vector<size_t>> result;
    if (std::filesystem::exists(path)) {
      std::ifstream input(path, std::ios::binary | std::ios::in);
      if (input.is_open()) {
        std::string line;
        while (std::getline(input, line)) {
          result.emplace_back();
          for (auto c : line) {
            std::string tmp;
            tmp += c;
            result.back().emplace_back(std::stoi(tmp));
          }
        }
      } else {
        std::cerr << "FileReader could not open file" << std::endl;
      }
    } else {
      std::cerr << "File " << path << " does not exist" << std::endl;
    }
    return result;
  }
};
}  // namespace fsu
