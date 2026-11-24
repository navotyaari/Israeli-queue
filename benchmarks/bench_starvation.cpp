#include <nanobench.h>
#include <israeli_queue/israeli_queue.hpp>
#include <optional>

// Counts dequeue calls before the sentinel value is returned.
// Correct starvation setup:
//   1. Seed K groups (one element each) — these sit ahead of the sentinel.
//   2. Enqueue the sentinel — unique key, goes to the back.
//   3. Enqueue N more elements that join the K existing front groups,
//      leaving the sentinel further and further behind (uncapped)
//      or bounded by M×K dequeues (capped).
template <typename Queue>
static int dequeues_until_sentinel(Queue& q, int sentinel) {
    int count = 0;
    while (true) {
        auto val = q.dequeue();
        ++count;
        if (val.has_value() && *val == sentinel) return count;
    }
}

static int sentinel_key(const int& v) { return (v == -999) ? -1 : v % 10; }

static int measure_sentinel_position(int k, int n, std::size_t cap) {
    iq::IsraeliQueue<int, int(*)(const int&)> q(sentinel_key, cap);
    for (int i = 0; i < k; ++i) q.enqueue(i);
    q.enqueue(-999);
    for (int i = k; i < k + n; ++i) q.enqueue(i);
    return dequeues_until_sentinel(q, -999);
}

void run_starvation_benchmarks(ankerl::nanobench::Bench& bench) {
    bench.title("Scenario 5: starvation latency (dequeues before lone task runs)");

    // Print sentinel dequeue position before timing to show starvation bounds.
    std::printf("\n-- Scenario 5: sentinel dequeue position (N=1000, K=10) --\n");
    std::printf("  uncapped:    sentinel served after %4d dequeues\n",
                measure_sentinel_position(10, 1000, 0));
    std::printf("  capped M=10: sentinel served after %4d dequeues  (bound = K*M = %d)\n",
                measure_sentinel_position(10, 1000, 10), 10 * 10);
    std::printf("  capped M=5:  sentinel served after %4d dequeues  (bound = K*M = %d)\n\n",
                measure_sentinel_position(10, 1000, 5), 10 * 5);

    bench.minEpochIterations(10).warmup(1);

    constexpr int N        = 1'000; // extra elements added to front groups
    constexpr int K        = 10;    // number of front groups
    constexpr int SENTINEL = -999;  // unique key value

    // key_fn: sentinel gets key -1 (unique), others get key 0..K-1
    // Uncapped: N elements all join their respective front groups -> sentinel
    // is served only after all K*(1 + N/K) front elements drain.
    bench.run("uncapped  N=1000 K=10", [&] {
        iq::IsraeliQueue<int, int(*)(const int&)> q(sentinel_key);
        for (int i = 0; i < K; ++i) q.enqueue(i);
        q.enqueue(SENTINEL);
        for (int i = K; i < K + N; ++i) q.enqueue(i);
        int d = dequeues_until_sentinel(q, SENTINEL);
        ankerl::nanobench::doNotOptimizeAway(d);
    });

    // Capped M=10: each front group caps at 10 -> overflow goes behind sentinel.
    // Sentinel exits after K*M = 10*10 = 100 dequeues.
    bench.run("capped M=10  N=1000 K=10", [&] {
        iq::IsraeliQueue<int, int(*)(const int&)> q(sentinel_key, 10);
        for (int i = 0; i < K; ++i) q.enqueue(i);
        q.enqueue(SENTINEL);
        for (int i = K; i < K + N; ++i) q.enqueue(i);
        int d = dequeues_until_sentinel(q, SENTINEL);
        ankerl::nanobench::doNotOptimizeAway(d);
    });

    // Capped M=5: sentinel exits after K*M = 10*5 = 50 dequeues.
    bench.run("capped M=5   N=1000 K=10", [&] {
        iq::IsraeliQueue<int, int(*)(const int&)> q(sentinel_key, 5);
        for (int i = 0; i < K; ++i) q.enqueue(i);
        q.enqueue(SENTINEL);
        for (int i = K; i < K + N; ++i) q.enqueue(i);
        int d = dequeues_until_sentinel(q, SENTINEL);
        ankerl::nanobench::doNotOptimizeAway(d);
    });
}
