//
// Created by schrodinger on 2/3/22.
//
#define MEMCPY_AMD64_PREFIX custom_

#include "../memcpy.cpp"
#include <vector>
#include <random>
#include <cstring>
#include <benchmark/benchmark.h>

namespace sse_only {
    extern "C" __attribute__((noinline)) void *
    memcpy_impl(void *__restrict dst_, const void *__restrict src_, size_t size) {
        /// We will use pointer arithmetic, so char pointer will be used.
        /// Note that __restrict makes sense (otherwise compiler will reload data from memory
        /// instead of using the value of registers due to possible aliasing).
        auto *__restrict dst = reinterpret_cast<std::byte *__restrict>(dst_);
        const auto *__restrict src = reinterpret_cast<const std::byte *__restrict>(src_);

        /// Standard memcpy returns the original value of dst. It is rarely used but we have to do it.
        /// If you use memcpy with small but non-constant sizes, you can call inline_memcpy directly
        /// for inlining and removing this single instruction.
        void *ret = dst;

        tail:
        /// Small sizes and tails after the loop for large sizes.
        /// The order of branches is important but in fact the optimal order depends on the distribution of sizes in your application.
        /// This order of branches is from the disassembly of glibc's code.
        /// We copy chunks of possibly uneven size with two overlapping movs.
        /// Example: to copy 5 bytes [0, 1, 2, 3, 4] we will copy tail [1, 2, 3, 4] first and then head [0, 1, 2, 3].
        if (size <= 16) {
            if (size >= 8) {
                /// Chunks of 8..16 bytes.
                __builtin_memcpy_inline(dst + size - 8, src + size - 8, 8);
                __builtin_memcpy_inline(dst, src, 8);
            } else if (size >= 4) {
                /// Chunks of 4..7 bytes.
                __builtin_memcpy_inline(dst + size - 4, src + size - 4, 4);
                __builtin_memcpy_inline(dst, src, 4);
            } else if (size >= 2) {
                /// Chunks of 2..3 bytes.
                __builtin_memcpy_inline(dst + size - 2, src + size - 2, 2);
                __builtin_memcpy_inline(dst, src, 2);
            } else if (size >= 1) {
                /// A single byte.
                *dst = *src;
            }
            /// No bytes remaining.
        } else {
            /// Medium and large sizes.
            if (size <= 128) {
                /// Medium size, not enough for full loop unrolling.

                /// We will copy the last 16 bytes.
                __builtin_memcpy_inline(dst + size - 16, src + size - 16, 16);

                /// Then we will copy every 16 bytes from the beginning in a loop.
                /// The last loop iteration will possibly overwrite some part of already copied last 16 bytes.
                /// This is Ok, similar to the code for small sizes above.
                while (size > 16) {
                    __builtin_memcpy_inline(dst, src, 16);
                    dst += 16;
                    src += 16;
                    size -= 16;
                }
            } else {
                /// Align destination to 16 bytes boundary.
                size_t padding = (-reinterpret_cast<size_t>(dst)) & 15;

                // avoid branch
                __builtin_memcpy_inline(dst, src, 16);
                dst += padding;
                src += padding;
                size -= padding;

                /// Aligned unrolled copy. We will use half of available SSE registers.
                /// It's not possible to have both src and dst aligned.
                /// So, we will use aligned stores and unaligned loads.
                __m128i c0, c1, c2, c3, c4, c5, c6, c7;

                while (size >= 128) {
                    c0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src) + 0);
                    c1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src) + 1);
                    c2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src) + 2);
                    c3 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src) + 3);
                    c4 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src) + 4);
                    c5 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src) + 5);
                    c6 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src) + 6);
                    c7 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src) + 7);
                    src += 128;
                    _mm_store_si128((reinterpret_cast<__m128i *>(dst) + 0), c0);
                    _mm_store_si128((reinterpret_cast<__m128i *>(dst) + 1), c1);
                    _mm_store_si128((reinterpret_cast<__m128i *>(dst) + 2), c2);
                    _mm_store_si128((reinterpret_cast<__m128i *>(dst) + 3), c3);
                    _mm_store_si128((reinterpret_cast<__m128i *>(dst) + 4), c4);
                    _mm_store_si128((reinterpret_cast<__m128i *>(dst) + 5), c5);
                    _mm_store_si128((reinterpret_cast<__m128i *>(dst) + 6), c6);
                    _mm_store_si128((reinterpret_cast<__m128i *>(dst) + 7), c7);
                    dst += 128;
                    size -= 128;
                }

                /// The latest remaining 0..127 bytes will be processed as usual.
                goto tail;
            }
        }
        return ret;
    }
}


std::vector<char> gen_data(size_t size) {
    std::random_device rdev;
    std::default_random_engine eng(rdev());
    std::uniform_int_distribution<char> dist;
    std::vector<char> data(size);
    for (auto &i: data) {
        i = dist(eng);
    }
    return data;
}

template<class Copy>
__attribute__((always_inline)) static inline void run(benchmark::State &state, const std::vector<size_t> &list, Copy copy) {
    std::vector<std::vector<char>> data;
    data.reserve(list.size());
    for (auto i: list) {
        data.push_back(gen_data(i));
    }
    std::vector data2 = data;
    for (auto _: state)
        for (int i = 0; i < data.size(); ++i)
            copy(data2[i].data(), data[i].data(), data2[i].size());
    size_t size_sum = 0;
    for (auto &i: data) {
        size_sum += data.size();
    }
    state.SetBytesProcessed(state.iterations() * size_sum);
}

static void memcpy_less_than_16_system(benchmark::State &state) {
    run(state, {1, 2, 3, 7, 11}, std::memcpy);
}

static void memcpy_less_than_16_custom(benchmark::State &state) {
    run(state, {1, 2, 3, 7, 11}, custom_memcpy);
}

static void memcpy_less_than_16_sse(benchmark::State &state) {
    run(state, {1, 2, 3, 7, 11}, sse_only::memcpy_impl);
}

static void memcpy_16_to_128_system(benchmark::State &state) {
    run(state, {16, 33, 99, 127}, std::memcpy);
}

static void memcpy_16_to_128_custom(benchmark::State &state) {
    run(state, {16, 33, 99, 127}, custom_memcpy);
}

static void memcpy_16_to_128_sse(benchmark::State &state) {
    run(state, {16, 33, 99, 127}, sse_only::memcpy_impl);
}

static void memcpy_128_to_2048_system(benchmark::State &state) {
    run(state, {128, 256, 1024, 2047}, std::memcpy);
}

static void memcpy_128_to_2048_custom(benchmark::State &state) {
    run(state, {128, 256, 1024, 2047}, custom_memcpy);
}

static void memcpy_128_to_2048_sse(benchmark::State &state) {
    run(state, {128, 256, 1024, 2047}, sse_only::memcpy_impl);
}

static void memcpy_2048_to_786432_system(benchmark::State &state) {
    run(state, {2048, 4096, 10000, 786431}, std::memcpy);
}

static void memcpy_2048_to_786432_custom(benchmark::State &state) {
    run(state, {2048, 4096, 10000, 786431}, custom_memcpy);
}

static void memcpy_2048_to_786432_sse(benchmark::State &state) {
    run(state, {2048, 4096, 10000, 786431}, sse_only::memcpy_impl);
}

static void memcpy_786432_to_12582912_system(benchmark::State &state) {
    run(state, {786432, 1572897, 6291467, 12582911}, std::memcpy);
}

static void memcpy_786432_to_12582912_custom(benchmark::State &state) {
    run(state, {786432, 1572897, 6291467, 12582911}, custom_memcpy);
}

static void memcpy_786432_to_12582912_sse(benchmark::State &state) {
    run(state, {786432, 1572897, 6291467, 12582911}, sse_only::memcpy_impl);
}

static void memcpy_above_12582912_system(benchmark::State &state) {
    run(state, {12582912, 25165824 + 511}, std::memcpy);
}

static void memcpy_above_12582912_custom(benchmark::State &state) {
    run(state, {12582912, 25165824 + 511}, custom_memcpy);
}

static void memcpy_above_12582912_sse(benchmark::State &state) {
    run(state, {12582912, 25165824 + 511}, sse_only::memcpy_impl);
}

static void memcpy_mixed_size_system(benchmark::State &state) {
    run(state, {1, 5, 16, 512, 5000, 12582913, 3000, 1000, 130, 786439}, std::memcpy);
}

static void memcpy_mixed_size_custom(benchmark::State &state) {
    run(state, {1, 5, 16, 512, 5000, 12582913, 3000, 1000, 130, 786439}, custom_memcpy);
}

static void memcpy_mixed_size_sse(benchmark::State &state) {
    run(state, {1, 5, 16, 512, 5000, 12582913, 3000, 1000, 130, 786439}, sse_only::memcpy_impl);
}

BENCHMARK(memcpy_mixed_size_system);
BENCHMARK(memcpy_mixed_size_custom);
BENCHMARK(memcpy_mixed_size_sse);

BENCHMARK(memcpy_less_than_16_system);
BENCHMARK(memcpy_less_than_16_custom);
BENCHMARK(memcpy_less_than_16_sse);

BENCHMARK(memcpy_16_to_128_system);
BENCHMARK(memcpy_16_to_128_custom);
BENCHMARK(memcpy_16_to_128_sse);

BENCHMARK(memcpy_128_to_2048_system);
BENCHMARK(memcpy_128_to_2048_custom);
BENCHMARK(memcpy_128_to_2048_sse);

BENCHMARK(memcpy_2048_to_786432_system);
BENCHMARK(memcpy_2048_to_786432_custom);
BENCHMARK(memcpy_2048_to_786432_sse);

BENCHMARK(memcpy_786432_to_12582912_system);
BENCHMARK(memcpy_786432_to_12582912_custom);
BENCHMARK(memcpy_786432_to_12582912_sse);

BENCHMARK(memcpy_above_12582912_system);
BENCHMARK(memcpy_above_12582912_custom);
BENCHMARK(memcpy_above_12582912_sse);
BENCHMARK_MAIN();