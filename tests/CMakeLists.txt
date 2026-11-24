include(CTest)
include(Catch)

add_executable(israeli_queue_tests
    test_core.cpp
    test_group_semantics.cpp
    test_starvation.cpp
    test_contract.cpp
)

target_link_libraries(israeli_queue_tests
    PRIVATE
        israeli_queue
        Catch2::Catch2WithMain
)

target_compile_options(israeli_queue_tests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

catch_discover_tests(israeli_queue_tests)
