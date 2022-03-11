#pragma once

#include <tbb/tbb.h>

namespace tbb_templates {

template <typename T>
void parallel_for(T& collection, std::function<void(size_t id)> func) {
  tbb::parallel_for(size_t(0), collection.size(), func);
}

template <typename T>
void parallel_for(T enum_start, T enum_end,
                  std::function<void(size_t id)> func) {
  tbb::parallel_for(static_cast<size_t>(enum_start),
                    static_cast<size_t>(enum_end), func);
}

template <typename T>
using concurrent_vector = tbb::concurrent_vector<T>;

}  // namespace tbb_templates
