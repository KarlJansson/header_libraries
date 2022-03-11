#pragma once

namespace ecs {
template <typename T>
using dsm = std::unordered_map<std::type_index, T>;

template <typename T>
class EntityComponents {
 public:
  EntityComponents(size_t size, std::function<T*(size_t)> func)
      : size(size), func(func) {}
  EntityComponents(const EntityComponents& copy) = delete;
  EntityComponents& operator=(const EntityComponents& copy) = delete;

  class iterator {
   public:
    iterator(size_t sub_loc, std::function<T*(size_t)> func)
        : sub_loc(sub_loc), func(func) {}

    auto operator++() {
      ++sub_loc;
      return *this;
    }
    bool operator!=(const iterator& other) { return other.sub_loc != sub_loc; }
    const auto operator*() { return func(sub_loc); }

   private:
    size_t sub_loc;
    std::function<T*(size_t)> func;
  };

  auto begin() { return iterator(0, func); }
  auto end() { return iterator(size, func); }

 private:
  size_t size;
  std::function<T*(size_t)> func;
};

template <typename EntMgr>
class Entity {
 public:
  Entity() : loc_map_(nullptr) {}
  Entity(EntMgr* ent_mgr)
      : ent_mgr_(ent_mgr),
        loc_map_(std::make_shared<dsm<std::vector<std::uint64_t>>>()) {}

  template <typename T, typename Ent>
  T* AddComponent() {
    return ent_mgr_->template AddComponent<T, Ent>(*this);
  }

  template <typename T>
  void RemoveComponent(std::uint64_t sub_loc = 0) {
    return ent_mgr_->template RemoveComponent<T>(*this, sub_loc);
  }

  template <typename T, typename Ent>
  const T* ComponentR(std::uint64_t sub_loc = 0) {
    return ent_mgr_->template ComponentR<T, Ent>(*this, sub_loc);
  }

  template <typename T, typename Ent>
  T* ComponentW(std::uint64_t sub_loc = 0) {
    return ent_mgr_->template ComponentW<T, Ent>(*this, sub_loc);
  }

  template <typename T>
  std::uint64_t ComponentCount() const {
    if (auto it = loc_map_->find(typeid(T)); it != std::end(*loc_map_))
      return it->second.size();
    return std::uint64_t(0);
  }

  template <typename T, typename Ent>
  EntityComponents<const T> ComponentsR() {
    return EntityComponents<const T>(
        ComponentCount<T>(), [this](size_t sub_loc) -> auto {
          return ComponentR<T, Ent>(sub_loc);
        });
  }

  template <typename T, typename Ent>
  EntityComponents<T> ComponentsW() {
    return EntityComponents<T>(
        ComponentCount<T>(), [this](size_t sub_loc) -> auto {
          return ComponentW<T, Ent>(sub_loc);
        });
  }

  bool operator<(const Entity& other) const {
    return loc_map_.get() < other.loc_map_.get();
  };
  bool operator==(const Entity& other) const {
    return loc_map_.get() == other.loc_map_.get();
  };

 private:
  template <typename T>
  std::uint64_t Loc(std::uint64_t sub_loc = 0) const {
    if (auto it = loc_map_->find(typeid(T));
        it != std::end(*loc_map_) && it->second.size() > sub_loc)
      return std::get<std::vector<std::uint64_t>>(*it)[sub_loc];
    return std::numeric_limits<std::uint64_t>::max();
  }

  EntMgr* ent_mgr_;
  std::shared_ptr<dsm<std::vector<std::uint64_t>>> loc_map_;

  friend class EntityManager;
  friend class EntityManagerMock;
};

}  // namespace ecs
