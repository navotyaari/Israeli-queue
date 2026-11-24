cmake_minimum_required(VERSION 3.14)
project(israeli_queue VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(israeli_queue INTERFACE)
target_include_directories(israeli_queue INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

option(IQ_BUILD_TESTS      "Build tests"      ON)
option(IQ_BUILD_BENCHMARKS "Build benchmarks" ON)

include(cmake/dependencies.cmake)

if(IQ_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(IQ_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
