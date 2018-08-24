// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/container/dense_set.h"

#include <chrono>
#include <random>
#include <set>
#include <string>

#include "benchmark/benchmark.h"

namespace {

class randomStreamUniformInt {
public:
    explicit randomStreamUniformInt(double seed, int lower_bound, int upper_bound)
        : mt(seed) , uniform_dist(lower_bound, upper_bound) {}
    int operator()() { return uniform_dist(mt); }
private:
    std::mt19937_64                     mt;
    std::uniform_int_distribution<>     uniform_dist;
};

std::vector<int> ConstructRandomData(const int range) {
    randomStreamUniformInt rng{double(range), -(1<<10), 1<<10};
    std::vector<int> data;
    data.reserve(range);
    for (int j = 0; j < range; ++j) {
       data.push_back(rng());
    }
    return data;
}

template<typename SET>
SET ConstructRandomSet(const int range) {
    auto data = ConstructRandomData(range);
    SET set(data.begin(), data.end());
    return set;
}

template<typename SET>
static void BM_SetConstruction(benchmark::State& state) {
    auto data = ConstructRandomData(state.range(0));
    SET set;
    for (auto _ : state) {
        set = SET(data.begin(), data.end());
    }
}
BENCHMARK_TEMPLATE(BM_SetConstruction, std::set<int>)->Ranges({{1<<8, 8<<8}});
BENCHMARK_TEMPLATE(BM_SetConstruction, absl::DenseSet<int>)->Ranges({{1<<8, 8<<8}});

template<typename SET>
static void BM_SetInsert(benchmark::State& state) {
    for (auto _ : state) {
      state.PauseTiming();
      SET set = ConstructRandomSet<SET>(state.range(0));
      auto data = ConstructRandomData(state.range(1));
      state.ResumeTiming();
      set.insert(data.begin(), data.end());
    }
}
BENCHMARK_TEMPLATE(BM_SetInsert, std::set<int>)->Ranges({{1<<8, 8<<8}, {128, 512}});
BENCHMARK_TEMPLATE(BM_SetInsert, absl::DenseSet<int>)->Ranges({{1<<8, 8<<8}, {128, 512}});

template<typename SET>
static void BM_SetCountRandom(benchmark::State& state) {
    SET set = ConstructRandomSet<SET>(state.range(0));
    auto data = ConstructRandomData(state.range(1));
    for (auto _ : state) {
        for (auto elem : data) {
            set.count(elem);
        }
    }
}
BENCHMARK_TEMPLATE(BM_SetCountRandom, std::set<int>)->Ranges({{4<<8, 8<<8}, {4<<8, 8<<8}});
BENCHMARK_TEMPLATE(BM_SetCountRandom, absl::DenseSet<int>)->Ranges({{4<<8, 8<<8}, {4<<8, 8<<8}});

template<typename SET>
static void BM_SetFindRandom(benchmark::State& state) {
    SET set = ConstructRandomSet<SET>(state.range(0));
    auto data = ConstructRandomData(state.range(1));
    for (auto _ : state) {
        for (auto elem : data) {
            set.find(elem);
        }
    }
}
BENCHMARK_TEMPLATE(BM_SetFindRandom, std::set<int>)->Ranges({{4<<8, 8<<8}, {1<<8, 8<<8}});
BENCHMARK_TEMPLATE(BM_SetFindRandom, absl::DenseSet<int>)->Ranges({{4<<8, 8<<8}, {1<<8, 8<<8}});

template<typename SET>
static void BM_SetFindExisting(benchmark::State& state) {
    SET set = ConstructRandomSet<SET>(state.range(0));
    set.insert(6);
    for (auto _ : state) {
        set.find(6);
    }
}
BENCHMARK_TEMPLATE(BM_SetFindExisting, std::set<int>)->Ranges({{4<<8, 8<<10}});
BENCHMARK_TEMPLATE(BM_SetFindExisting, absl::DenseSet<int>)->Ranges({{4<<8, 8<<10}});

template<typename SET>
static void BM_SetFindNonExisting(benchmark::State& state) {
    SET set = ConstructRandomSet<SET>(state.range(0));
    set.erase(6);
    for (auto _ : state) {
        set.find(6);
    }
}
BENCHMARK_TEMPLATE(BM_SetFindNonExisting, std::set<int>)->Ranges({{4<<8, 8<<10}});
BENCHMARK_TEMPLATE(BM_SetFindNonExisting, absl::DenseSet<int>)->Ranges({{4<<8, 8<<10}});


template<typename SET>
static void BM_SetSweep(benchmark::State& state) {
    SET set = ConstructRandomSet<SET>(state.range(0));
    for (auto _ : state) {
        for (auto it = set.begin(); it != set.end(); ++it){
            *it;
        }
    }
}
BENCHMARK_TEMPLATE(BM_SetSweep, std::set<int>)->Ranges({{4<<8, 8<<10}});
BENCHMARK_TEMPLATE(BM_SetSweep, absl::DenseSet<int>)->Ranges({{4<<8, 8<<10}});
}  // namespace
