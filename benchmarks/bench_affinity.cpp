#include <nanobench.h>
#include <israeli_queue/israeli_queue.hpp>
#include <chrono>
#include <queue>
#include <thread>
#include <vector>

// Simulates a workload where switching groups incurs a cold-start penalty.
// The penalty is kept very short so the benchmark finishes in reasonable time.
static constexpr auto COLD_START = std::chrono::microseconds(10);
static constexpr int  N          = 500;
static constexpr int  K          = 10;

static void process_with_affinity(int value, int& last_key) {
    int key = value % K;
    if (key != last_key) {
        std::this_thread::sleep_for(COLD_START);
        last_key = key;
    }
    ankerl::nanobench::doNotOptimizeAway(value);
}

void run_affinity_benchmarks(ankerl::nanobench::Bench& bench) {
    bench.title("Scenario 4: resource affinity (cold-start penalty per group switch)");
    // Run only once per epoch — sleep-based, not throughput.
    bench.minEpochIterations(1).warmup(0);

    // Build a randomly interleaved input (round-robin across keys)
    std::vector<int> input;
    input.reserve(N);
    for (int i = 0; i < N; ++i) input.push_back(i); // key = i % K

    bench.run("std::queue (interleaved order)  N=500 K=10", [&] {
        std::queue<int> q;
        for (int v : input) q.push(v);
        int last_key = -1;
        while (!q.empty()) {
            process_with_affinity(q.front(), last_key);
            q.pop();
        }
    });

    bench.run("IsraeliQueue (grouped order)    N=500 K=10", [&] {
        iq::IsraeliQueue<int, int(*)(const int&)> q([](const int& v) { return v % K; });
        for (int v : input) q.enqueue(v);
        int last_key = -1;
        while (!q.empty()) {
            auto val = q.dequeue();
            process_with_affinity(*val, last_key);
        }
    });
}
