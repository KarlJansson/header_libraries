#include "chunk_list.hpp"

using namespace synchronization;

void BM_chunklist(benchmark::State& state) {
  ChunkList<int, 10> chunk_list;
  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
    chunk_list.push_front(2);
    chunk_list.pop_back();
  }
}
// BENCHMARK(BM_chunklist)->ThreadPerCpu();
BENCHMARK(BM_chunklist)->DenseThreadRange(1, 1);
