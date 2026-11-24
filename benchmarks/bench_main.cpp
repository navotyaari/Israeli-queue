#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

void run_throughput_benchmarks(ankerl::nanobench::Bench&);
void run_pipeline_benchmarks(ankerl::nanobench::Bench&);
void run_affinity_benchmarks(ankerl::nanobench::Bench&);
void run_starvation_benchmarks(ankerl::nanobench::Bench&);

int main() {
    ankerl::nanobench::Bench bench;
    bench.warmup(1).minEpochIterations(100);

    run_throughput_benchmarks(bench);
    run_pipeline_benchmarks(bench);
    run_affinity_benchmarks(bench);
    run_starvation_benchmarks(bench);
}
