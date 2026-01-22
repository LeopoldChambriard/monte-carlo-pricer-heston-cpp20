cmake_minimum_required(VERSION 3.20)
project(monte_carlo_pricer_heston_cpp20 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

include(FetchContent)

# 1. Bibliothèque Pricer Core
add_library(pricer_core
    src/BlackScholesAnalytic.cpp
    src/PathGenerator.cpp
    src/MonteCarloEngine.cpp
)

target_include_directories(pricer_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 2. Tests unitaires (GoogleTest)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(test_pricer tests/test_pricer.cpp)
target_link_libraries(test_pricer PRIVATE pricer_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_pricer)

# 3. Benchmarks de débit (Google Benchmark)
FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(bench_pricer benchmarks/bench_pricer.cpp)
target_link_libraries(bench_pricer PRIVATE pricer_core benchmark::benchmark)
