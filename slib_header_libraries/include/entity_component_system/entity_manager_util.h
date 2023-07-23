#pragma once

namespace ecs {
template <typename T>
using dsm = std::unordered_map<std::type_index, T>;

template <typename T>
class EntityComponents {
 public:
  EntityComponents(size_t size, std::function<T*(size_t)> func)
      : size_(size), func(func) {}
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
    auto& operator*() { return *func(sub_loc); }

   private:
    size_t sub_loc;
    std::function<T*(size_t)> func;
  };

  auto begin() { return iterator(0, func); }
  auto end() { return iterator(size_, func); }

  auto size() { return size_; }
  auto empty() { return size_ == 0; }

  auto operator[](size_t i) { return *func(i); }

 private:
  size_t size_;
  std::function<T*(size_t)> func;
};

template <typename T, typename Ent>
class RemovedComponentsHolder {
 public:
  RemovedComponentsHolder(const std::vector<T>* comps, std::vector<Ent>* ents,
                          std::vector<size_t>* comp_locs)
      : components(comps), entities(ents), component_locs(comp_locs) {}
  RemovedComponentsHolder& operator=(const RemovedComponentsHolder& copy) =
      delete;

  template <typename T1>
  class iterator {
   public:
    iterator(T1 comps, std::vector<Ent>* ents, size_t* loc)
        : components(comps), entities(ents), loc(loc) {}

    auto operator++() {
      loc++;
      return *this;
    }

    bool operator!=(const iterator& other) { return other.loc != loc; }

    auto operator*() {
      return std::make_tuple(std::ref((*components)[*loc]),
                             std::ref((*entities)[*loc]));
    }

   private:
    T1 components;
    std::vector<Ent>* entities;
    size_t* loc;
  };

  auto begin() {
    if (components)
      return iterator<const std::vector<T>*>(components, entities,
                                             component_locs->data());
    return iterator<const std::vector<T>*>(nullptr, nullptr, nullptr);
  }

  auto end() {
    if (components)
      return iterator<const std::vector<T>*>(
          components, entities,
          component_locs->data() + component_locs->size());
    return iterator<const std::vector<T>*>(nullptr, nullptr, nullptr);
  }

  auto size() {
    if (component_locs) return component_locs->size();
    return std::size_t(0);
  }

  auto empty() {
    if (component_locs) return component_locs->size() == 0;
    return true;
  }

  auto operator[](size_t i) {
    return std::make_tuple(std::ref((*components)[(*component_locs)[i]]),
                           std::ref((*entities)[(*component_locs)[i]]));
  }

  const std::vector<T>* components;
  std::vector<Ent>* entities;
  std::vector<size_t>* component_locs;
};

template <typename Ent>
class EntityHolder {
 public:
  EntityHolder(std::vector<Ent>* ents) : entities(ents) {}
  EntityHolder& operator=(const EntityHolder& copy) = delete;

  class iterator {
   public:
    iterator(Ent* ent) : entity(ent) {}

    auto operator++() {
      ++entity;
      return *this;
    }

    bool operator!=(const iterator& other) { return other.entity != entity; }

    auto& operator*() { return *entity; }

   private:
    Ent* entity;
  };

  auto begin() {
    if (entities) return iterator(entities->data());
    return iterator(nullptr);
  }

  auto end() {
    if (entities) return iterator(entities->data() + entities->size());
    return iterator(nullptr);
  }

  auto size() {
    if (entities) return entities->size();
    return std::size_t(0);
  }

  auto empty() {
    if (entities) return entities->size() == 0;
    return true;
  }

  auto operator[](size_t i) { return (*entities)[i]; }

  std::vector<Ent>* entities{nullptr};
};

template <typename T, typename Ent>
class Components {
 public:
  Components(std::vector<T>* comps, std::vector<Ent>* ents)
      : components(comps), entities(ents) {}
  Components& operator=(const Components& copy) = delete;

  template <typename T1, typename T2>
  class iterator {
   public:
    iterator(T1* comp, T2* ent) : component(comp), entity(ent) {}

    auto operator++() {
      ++component;
      ++entity;
      return *this;
    }
    bool operator!=(const iterator& other) {
      return other.component != component;
    }
    auto operator*() {
      return std::make_tuple(std::ref(*component), std::ref(*entity));
    }

   private:
    T1* component;
    T2* entity;
  };

  auto begin() {
    if (components) return iterator(components->data(), entities->data());
    return iterator<T, Ent>(nullptr, nullptr);
  }

  auto end() {
    if (components)
      return iterator(components->data() + components->size(),
                      entities->data() + entities->size());
    return iterator<T, Ent>(nullptr, nullptr);
  }

  auto size() {
    if (components && entities)
      return std::min(components->size(), entities->size());
    return std::size_t(0);
  }

  auto empty() {
    if (components && entities)
      return std::min(components->size(), entities->size()) == 0;
    return true;
  }

  auto operator[](size_t i) {
    return std::make_tuple(std::ref((*components)[i]),
                           std::ref((*entities)[i]));
  }

  std::vector<T>* components{nullptr};
  std::vector<Ent>* entities{nullptr};
};

template <typename T, typename Ent>
class ConstComponents {
 public:
  ConstComponents(const std::vector<T>* comps, std::vector<Ent>* ents)
      : components(comps), entities(ents) {}
  ConstComponents& operator=(const ConstComponents& copy) = delete;

  template <typename T1, typename T2>
  class iterator {
   public:
    iterator(T1* comp, T2* ent) : component(comp), entity(ent) {}

    auto operator++() {
      ++component;
      ++entity;
      return *this;
    }
    bool operator!=(const iterator& other) {
      return other.component != component;
    }
    auto operator*() {
      return std::make_tuple(std::ref(*component), std::ref(*entity));
    }

   private:
    T1* component;
    T2* entity;
  };

  auto begin() {
    if (components) return iterator(components->data(), entities->data());
    return iterator<const T, Ent>(nullptr, nullptr);
  }

  auto end() {
    if (components)
      return iterator(components->data() + components->size(),
                      entities->data() + entities->size());
    return iterator<const T, Ent>(nullptr, nullptr);
  }

  auto size() {
    if (components && entities)
      return std::min(components->size(), entities->size());
    return std::size_t(0);
  }

  auto empty() {
    if (components && entities)
      return std::min(components->size(), entities->size()) == 0;
    return true;
  }

  auto operator[](size_t i) {
    return std::make_tuple(std::ref((*components)[i]),
                           std::ref((*entities)[i]));
  }

  const std::vector<T>* components{nullptr};
  std::vector<Ent>* entities{nullptr};
};

template <typename T, typename Ent>
class UpdatedComponents {
 public:
  UpdatedComponents(T comps, std::vector<Ent>* ents, std::vector<size_t>* inds)
      : components(comps), entities(ents), indices(inds) {}
  UpdatedComponents& operator=(const UpdatedComponents& copy) = delete;

  template <typename T1, typename T2>
  class iterator {
   public:
    iterator(T1 comp, T2 ent, size_t* inds)
        : component(comp), entity(ent), ind_iterator(inds) {}

    auto operator++() {
      ind_iterator++;
      return *this;
    }

    bool operator!=(const iterator& other) {
      return other.ind_iterator != ind_iterator;
    }

    auto operator*() {
      return std::make_tuple(std::ref((*component)[*ind_iterator]),
                             std::ref((*entity)[*ind_iterator]));
    }

   private:
    T1 component;
    T2 entity;
    size_t* ind_iterator;
  };

  auto begin() {
    if (components)
      return iterator<T, std::vector<Ent>*>(components, entities,
                                            indices->data());
    return iterator<T, std::vector<Ent>*>(nullptr, nullptr, nullptr);
  }

  auto end() {
    if (components)
      return iterator<T, std::vector<Ent>*>(components, entities,
                                            indices->data() + indices->size());
    return iterator<T, std::vector<Ent>*>(nullptr, nullptr, nullptr);
  }

  auto size() {
    if (indices) return indices->size();
    return std::size_t(0);
  }

  auto empty() {
    if (indices) return indices->size() == 0;
    return true;
  }

  auto operator[](size_t i) {
    return std::make_tuple(std::ref((*components)[(*indices)[i]]),
                           std::ref((*entities)[(*indices)[i]]));
  }

  T components;
  std::vector<Ent>* entities;
  std::vector<size_t>* indices;
};
}  // namespace ecs

namespace ecss {
template <typename T>
class EntityComponents {
 public:
  EntityComponents(tbb::concurrent_vector<std::any>* content)
      : content_(content) {}
  EntityComponents& operator=(const EntityComponents& copy) = delete;

  class iterator {
   public:
    iterator() {}
    iterator(tbb::concurrent_vector<std::any>::iterator it) : it_(it) {}

    auto operator++() {
      it_++;
      return *this;
    }

    bool operator!=(const iterator& other) { return other.it_ != it_; }

    auto& operator*() {
      auto& entry = std::any_cast<
          typename tbb::concurrent_vector<std::pair<T, Entity>>::iterator&>(
          *it_);
      return entry->first;
    }

   private:
    tbb::concurrent_vector<std::any>::iterator it_;
  };

  auto begin() {
    if (content_) return iterator(std::begin(*content_));
    return iterator();
  }

  auto end() {
    if (content_) return iterator(std::end(*content_));
    return iterator();
  }

  auto size() {
    if (content_) return content_->size();
    return size_t(0);
  }

  auto empty() {
    if (content_) return content_->empty();
    return true;
  }

  auto& operator[](size_t i) {
    auto entry = std::any_cast<
        typename tbb::concurrent_vector<std::pair<T, Entity>>::iterator&>(
        (*content_)[i]);
    return entry->first;
  }

 private:
  tbb::concurrent_vector<std::any>* content_;
};

template <typename T>
using Internal = tbb::concurrent_vector<std::pair<T, Entity>>;

template <typename T>
class ComponentHolder {
 public:
  ComponentHolder(Internal<T>* content) : content_(content) {}
  ComponentHolder& operator=(const ComponentHolder& copy) = delete;

  class iterator {
   public:
    iterator() {}
    iterator(typename Internal<T>::iterator it) : it_(it) {}

    auto operator++() {
      it_++;
      return *this;
    }

    bool operator!=(const iterator& other) { return other.it_ != it_; }

    auto operator*() {
      return std::make_tuple(std::ref(it_->first), std::ref(it_->second));
    }

   private:
    typename Internal<T>::iterator it_;
  };

  auto begin() {
    if (content_) return iterator(std::begin(*content_));
    return iterator();
  }

  auto end() {
    if (content_) return iterator(std::end(*content_));
    return iterator();
  }

  auto size() {
    if (content_) return content_->size();
    return size_t(0);
  }

  auto empty() {
    if (content_) return content_->empty();
    return true;
  }

  auto operator[](size_t i) {
    auto& entry = (*content_)[i];
    return std::make_tuple(std::ref(entry.first), std::ref(entry.second));
  }

 private:
  Internal<T>* content_;
};
}  // namespace ecss
