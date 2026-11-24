add_executable(israeli_queue_benchmarks
    bench_main.cpp
    bench_throughput.cpp
    bench_pipeline.cpp
    bench_affinity.cpp
    bench_starvation.cpp
)

target_link_libraries(israeli_queue_benchmarks
    PRIVATE
        israeli_queue
        nanobench
)
