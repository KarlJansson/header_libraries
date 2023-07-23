#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

namespace synchronization {
template <typename T, size_t S = 10>
class ChunkList {
 public:
  ChunkList() { chunk_ = std::make_unique<Chunk>(); }

  bool empty() { return num_elements_ == 0; }

  T* push_front(T c) {
    size_t num_elem = num_elements_++;
    size_t chunk_count = num_elem / S;

    Chunk* chunk{nullptr};
    do {
      chunk = chunk_.get();
      for (size_t i = 0; i < chunk_count; ++i) {
        chunk = chunk->next;
        if (!chunk) break;
      }
    } while (!chunk);

    if (num_elem % S == S / 2 && !chunk->next) {
      chunk->chunk = std::make_unique<Chunk>();
      chunk->next = chunk->chunk.get();
    }

    auto& be = chunk->buffer[(tail_ + num_elem) % S];
    be = c;
    return &be;
  }

  T* back() {
    if (empty()) return nullptr;
    Chunk* chunk = chunk_.get();
    int chunk_count = tail_ / S;
    for (int i = 0; i < chunk_count; ++i) chunk = chunk->next;
    return &chunk->buffer[tail_ % S];
  }

  void pop_back() {
    if (empty()) return;
    ++tail_;
    --num_elements_;
    if (empty()) tail_ = 0;
  }

 private:
  struct Chunk {
    T buffer[S];
    std::unique_ptr<Chunk> chunk;
    Chunk* next{nullptr};
  };

  std::unique_ptr<Chunk> chunk_;

  size_t tail_{0};
  std::atomic<size_t> num_elements_{0};
};

template <typename T, size_t CHUNK_SIZE = 10>
class ChunkList2 {
 public:
  ChunkList2() { AllocateChunk(); }

  struct Chunk {
    std::array<T, CHUNK_SIZE> elements;
    std::uint8_t num_elements{0};
    Chunk* next_chunk{nullptr};
  };
  using chunk_store_t = std::vector<std::unique_ptr<Chunk>>;

  class iterator {};

 private:
  void AllocateChunk() {
    auto next_chunk = std::make_unique<Chunk>();
    if (!chunk_store.empty()) chunk_store.back()->next_chunk = next_chunk.get();
    chunk_store.emplace_back(std::move(next_chunk));
  }

  chunk_store_t chunk_store;
  std::uint8_t num_elements{0};
};
}  // namespace synchronization
