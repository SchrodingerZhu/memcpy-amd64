cmake_minimum_required(VERSION 3.21)
project(memcpy_amd64)

set(CMAKE_CXX_STANDARD 17)

add_library(memcpy_amd64 memcpy.cpp)

set(MEMCPY_AMD64_FREESTANDING -nostdlib -ffreestanding -nodefaultlibs -fno-exceptions -fno-rtti)
set(MEMCPY_AMD64_WARNING_HIGH -Wall -Wextra -Weffc++ -Werror)
set(MEMCPY_AMD64_HIDDEN_VISIBILITY -fvisibility=hidden -fvisibility-inlines-hidden)
set(MEMCPY_AMD64_PREFIX custom_ CACHE STRING "prefix for memcpy symbol")

target_compile_options(memcpy_amd64 PRIVATE
        ${MEMCPY_AMD64_FREESTANDING} ${MEMCPY_AMD64_WARNING_HIGH} ${MEMCPY_AMD64_HIDDEN_VISIBILITY})
target_link_options(memcpy_amd64 PRIVATE
        ${MEMCPY_AMD64_FREESTANDING} ${MEMCPY_AMD64_WARNING_HIGH} ${MEMCPY_AMD64_HIDDEN_VISIBILITY})

option(MEMCPY_AMD64_ENABLE_BENCHMARK "enable benchmark" ON)
option(MEMCPY_AMD64_ENABLE_TEST "enable test" ON)

if(MEMCPY_AMD64_ENABLE_TEST)
    add_subdirectory(contrib/googletest)
    add_executable(memcpy_test test/test.cpp)
    target_link_libraries(memcpy_test PRIVATE gtest_main)
endif()

if(MEMCPY_AMD64_ENABLE_BENCHMARK)
    add_subdirectory(contrib/benchmark)
    add_executable(memcpy_bench bench/benchmark.cpp)
    target_link_libraries(memcpy_bench PRIVATE benchmark::benchmark)
endif()
