#pragma once

#include <any>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "../tbb_templates.hpp"

namespace ecss {
class Entity {
 public:
  using Internal =
      tbb::concurrent_unordered_map<size_t, tbb::concurrent_vector<std::any>>;
  Entity() : loc_(nullptr) {}
  Entity(int) : loc_(std::make_shared<Internal>()) {}

  bool operator<(const Entity& other) const {
    return loc_.get() < other.loc_.get();
  };
  bool operator>(const Entity& other) const {
    return loc_.get() > other.loc_.get();
  };
  bool operator==(const Entity& other) const {
    return loc_.get() == other.loc_.get();
  };
  bool operator!=(const Entity& other) const {
    return loc_.get() != other.loc_.get();
  };

  std::shared_ptr<Internal> loc_;
};

}  // namespace ecss

namespace ecs {
template <typename T>
using dsm = std::unordered_map<std::type_index, T>;

class Entity {
 public:
  Entity() : loc_map_(nullptr) {}
  Entity(int) : loc_map_(std::make_shared<dsm<std::vector<std::uint64_t>>>()) {}

  bool operator<(const Entity& other) const {
    return loc_map_.get() < other.loc_map_.get();
  };
  bool operator>(const Entity& other) const {
    return loc_map_.get() > other.loc_map_.get();
  };
  bool operator==(const Entity& other) const {
    return loc_map_.get() == other.loc_map_.get();
  };
  bool operator!=(const Entity& other) const {
    return loc_map_.get() != other.loc_map_.get();
  };

  template <typename T>
  std::uint64_t Loc(std::uint64_t sub_loc = 0) const {
    if (auto it = loc_map_->find(typeid(T));
        it != std::end(*loc_map_) && it->second.size() > sub_loc)
      return std::get<std::vector<std::uint64_t>>(*it)[sub_loc];
    return std::numeric_limits<std::uint64_t>::max();
  }

  std::shared_ptr<dsm<std::vector<std::uint64_t>>> loc_map_;
};
}  // namespace ecs
