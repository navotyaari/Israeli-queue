#include <nanobench.h>
#include <israeli_queue/israeli_queue.hpp>
#include <queue>
#include <unordered_map>
#include <vector>

void run_pipeline_benchmarks(ankerl::nanobench::Bench& bench) {
    bench.title("Scenario 3: grouped processing pipeline");

    constexpr int N = 10'000;
    constexpr int K = 100;

    // Approach A: std::queue + unordered_map accumulation pass
    bench.run("std::queue + accumulation pass  N=10k K=100", [&] {
        std::queue<int> q;
        for (int i = 0; i < N; ++i) q.push(i);

        std::unordered_map<int, std::vector<int>> groups;
        groups.reserve(K);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            groups[v % K].push_back(v);
        }
        for (auto& [key, vec] : groups) {
            for (int v : vec) ankerl::nanobench::doNotOptimizeAway(v);
        }
    });

    // Approach B: IsraeliQueue — grouping is free, dequeue order is grouped
    bench.run("IsraeliQueue inline grouping     N=10k K=100", [&] {
        iq::IsraeliQueue<int, int(*)(const int&)> q([](const int& v) { return v % 100; });
        for (int i = 0; i < N; ++i) q.enqueue(i);
        while (!q.empty()) ankerl::nanobench::doNotOptimizeAway(q.dequeue());
    });
}
