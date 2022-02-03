cmake_minimum_required(VERSION 3.21)
project(memcpy_amd64)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_TESTING OFF CACHE BOOL "disable cpu feature tests" FORCE)

add_library(memcpy-amd64 INTERFACE)
add_library(memcpy-amd64-shared SHARED src/memcpy.cpp)
add_library(memcpy-amd64-static STATIC src/memcpy.cpp)
add_library(memcpy-amd64-object OBJECT src/memcpy.cpp)

set(MEMCPY_AMD64_FREESTANDING -nostdlib -ffreestanding -nodefaultlibs -fno-exceptions -fno-rtti)
set(MEMCPY_AMD64_WARNING_HIGH -Wall -Wextra -Weffc++ -Werror)
set(MEMCPY_AMD64_HIDDEN_VISIBILITY -fvisibility=hidden -fvisibility-inlines-hidden)
set(MEMCPY_AMD64_PREFIX custom_ CACHE STRING "prefix for memcpy symbol")

target_compile_options(memcpy-amd64-shared PRIVATE
        ${MEMCPY_AMD64_FREESTANDING} ${MEMCPY_AMD64_WARNING_HIGH} ${MEMCPY_AMD64_HIDDEN_VISIBILITY})
target_link_options(memcpy-amd64-shared PRIVATE
        ${MEMCPY_AMD64_FREESTANDING} ${MEMCPY_AMD64_WARNING_HIGH} ${MEMCPY_AMD64_HIDDEN_VISIBILITY})

option(MEMCPY_AMD64_ENABLE_BENCHMARK "enable benchmark" ON)
option(MEMCPY_AMD64_ENABLE_TEST "enable test" ON)

if(MEMCPY_AMD64_ENABLE_TEST)
    add_subdirectory(contrib/googletest)
    add_executable(memcpy_test test/test.cpp)
    target_link_libraries(memcpy_test PRIVATE gtest_main)

    add_executable(memcpy_main test/test_basic.cpp)
    target_link_libraries(memcpy_main memcpy-amd64-shared)

    enable_testing()
    add_test(memcpy_gtest memcpy_test)
    add_test(memcpy_basic memcpy_main)
endif()

if(MEMCPY_AMD64_ENABLE_BENCHMARK)
    add_subdirectory(contrib/benchmark)
    add_executable(memcpy_bench bench/benchmark.cpp)
    target_link_libraries(memcpy_bench PRIVATE benchmark::benchmark)
endif()

configure_file(include/memcpy-amd64/memcpy.h.in include/memcpy-amd64/memcpy.h)
target_include_directories(memcpy-amd64 INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/include include)

target_link_libraries(memcpy-amd64-shared memcpy-amd64)
target_link_libraries(memcpy-amd64-static memcpy-amd64)
target_link_libraries(memcpy-amd64-object memcpy-amd64)
