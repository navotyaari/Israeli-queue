#include <nanobench.h>
#include <israeli_queue/israeli_queue.hpp>
#include <queue>

template <typename KeyFn>
static void bench_iq(ankerl::nanobench::Bench& bench,
                     const char* name, int n, KeyFn key_fn) {
    bench.run(name, [&] {
        iq::IsraeliQueue<int, KeyFn> q(key_fn);
        for (int i = 0; i < n; ++i) q.enqueue(i);
        while (!q.empty()) ankerl::nanobench::doNotOptimizeAway(q.dequeue());
    });
}

static void bench_std_queue(ankerl::nanobench::Bench& bench,
                             const char* name, int n) {
    bench.run(name, [&] {
        std::queue<int> q;
        for (int i = 0; i < n; ++i) q.push(i);
        while (!q.empty()) {
            ankerl::nanobench::doNotOptimizeAway(q.front());
            q.pop();
        }
    });
}

void run_throughput_benchmarks(ankerl::nanobench::Bench& bench) {
    bench.title("Throughput: IsraeliQueue vs std::queue");

    constexpr int N = 10'000;

    // Scenario 1: degenerate — K == N (every element its own group)
    bench_std_queue(bench, "std::queue          N=10k",        N);
    bench_iq(bench,        "IsraeliQueue K=N    N=10k", N, [](const int& v) { return v; });

    // Scenario 2: high clustering — K << N
    bench_iq(bench, "IsraeliQueue K=2    N=10k", N, [](const int& v) { return v % 2; });
    bench_iq(bench, "IsraeliQueue K=10   N=10k", N, [](const int& v) { return v % 10; });
    bench_iq(bench, "IsraeliQueue K=100  N=10k", N, [](const int& v) { return v % 100; });
}
