//
// Created by schrodinger on 2/3/22.
//
#pragma once

#include <cpuid.h>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <immintrin.h>
#include <array>

namespace memcpy_amd64 {

    namespace detail {
        __attribute__((always_inline)) static inline
        bool rep_movsb(void *__restrict dst, const void *__restrict src, size_t size) {
            asm volatile("rep movsb"
            : "+D"(dst), "+S"(src), "+c"(size)
            :
            : "memory");
            return dst;
        }

        __attribute__((noinline, target("avx2"))) static inline void memcpy_avx2_page2(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size);

        __attribute__((noinline, target("avx2"))) static inline void memcpy_avx2_page4(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size);
    }

    namespace config {
        static inline constexpr size_t page_size = 0x1000;
        static inline constexpr size_t erms_lower_bound = 2048;
        static inline constexpr size_t non_temporal_lower_bound = 0xc0000;
    };

    namespace vectorize {
        struct V32 {
            using vector = __m256i;

            __attribute__((always_inline, target("avx2"))) static inline vector aligned_load(const void *address) {
                return _mm256_load_si256(static_cast<const vector *>(address));
            }

            __attribute__((always_inline, target("avx2"))) static inline vector unaligned_load(const void *address) {
                return _mm256_loadu_si256(static_cast<const vector *>(address));
            }

            __attribute__((always_inline, target("avx2"))) static inline vector nt_load(const void *address) {
                return _mm256_stream_load_si256(static_cast<vector *>(const_cast<void *>(address)));
            }

            __attribute__((always_inline, target("avx2"))) static inline void
            aligned_store(void *address, const vector &val) {
                return _mm256_store_si256(static_cast<vector *>(address), val);
            }

            __attribute__((always_inline, target("avx2"))) static inline void
            unaligned_store(void *address, const vector &val) {
                return _mm256_storeu_si256(static_cast<vector *>(address), val);
            }

            __attribute__((always_inline, target("avx2"))) static inline void
            nt_store(void *address, const vector &val) {
                return _mm256_stream_si256(static_cast<vector *>(address), val);
            }
        };
    }

    __attribute__((always_inline)) static inline void *
    inline_memcpy(void *__restrict dst_, const void *__restrict src_, size_t size) {
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
            if (__builtin_expect(size <= 128, 1)) {
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
                auto large_body = [&]() __attribute__((noinline)) {

                    if (__builtin_expect(size >= config::non_temporal_lower_bound && __builtin_cpu_supports("avx2"),
                                         0)) { //
                        if (size >= 16 * config::non_temporal_lower_bound) {
                            detail::memcpy_avx2_page4(dst, src, size);
                        } else {
                            detail::memcpy_avx2_page2(dst, src, size);
                        }
                        if (size <= 128) return false;
                    }

                    if (__builtin_expect(size >= config::erms_lower_bound, 0)) {
                        detail::rep_movsb(dst, src, size);
                        return true;
                    }

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
                    return false;
                };

                if (!large_body()) goto tail;
                /// The latest remaining 0..127 bytes will be processed as usual.
            }
        }
        return ret;
    }


    namespace detail {
        template<size_t page_num, size_t vec_num, typename Load>
        __attribute__((always_inline, target("avx2"))) void memcpy_avx2_impl(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size,
                Load load) {
            using namespace vectorize;
            using vector = V32::vector;
            constexpr size_t stride_size = vec_num * sizeof(vector);
            std::array<vector, vec_num * page_num> storage{};

            while (size >= page_num * config::page_size) {
                for (size_t i = 0; i < config::page_size / stride_size; ++i) {
                    auto target = static_cast<std::byte *>(__builtin_assume_aligned(dst, alignof(vector)));
#pragma clang loop unroll(full)
                    for (size_t p = 0; p < page_num; ++p) {
                        __builtin_prefetch(src + config::page_size * p + stride_size);
                    };

#pragma clang loop unroll(full)
                    for (size_t p = 0; p < page_num; ++p) {
#pragma clang loop unroll(full)
                        for (size_t v = 0; v < vec_num; ++v) {
                            storage[p * 4 + v] = load(src + config::page_size * p + sizeof(vector) * v);
                        };
                    };

#pragma clang loop unroll(full)
                    for (size_t p = 0; p < page_num; ++p) {
#pragma clang loop unroll(full)
                        for (size_t v = 0; v < vec_num; ++v) {
                            V32::nt_store(target + config::page_size * p + sizeof(vector) * v, storage[p * 4 + v]);
                        }
                    }
                    dst += stride_size;
                    src += stride_size;
                }
                dst += (page_num - 1) * config::page_size;
                src += (page_num - 1) * config::page_size;
                size -= page_num * config::page_size;
            }
        }

        __attribute__((noinline, target("avx2"))) static inline void memcpy_avx2_page4(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size) {
            using namespace vectorize;
            using vector = V32::vector;
            auto dst_padding = (-reinterpret_cast<uintptr_t>(dst)) & (sizeof(vector) - 1);
            auto diff = (reinterpret_cast<uintptr_t>(dst) ^ reinterpret_cast<uintptr_t>(src)) & (sizeof(vector) - 1);

            __builtin_memcpy_inline(dst, src, sizeof(vector));
            dst += dst_padding;
            src += dst_padding;
            size -= dst_padding;

            if (__builtin_expect(diff == 0, 0)) {
                memcpy_avx2_impl<4, 4>(dst, src, size, V32::aligned_load);
            } else {
                memcpy_avx2_impl<4, 4>(dst, src, size, V32::unaligned_load);
            }
        }

        __attribute__((noinline, target("avx2"))) static inline void memcpy_avx2_page2(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size) {
            using namespace vectorize;
            using vector = V32::vector;
            auto dst_padding = (-reinterpret_cast<uintptr_t>(dst)) & (sizeof(vector) - 1);
            auto diff = (reinterpret_cast<uintptr_t>(dst) ^ reinterpret_cast<uintptr_t>(src)) & (sizeof(vector) - 1);

            __builtin_memcpy_inline(dst, src, sizeof(vector));
            dst += dst_padding;
            src += dst_padding;
            size -= dst_padding;


            if (__builtin_expect(diff == 0, 0)) {
                memcpy_avx2_impl<2, 4>(dst, src, size, V32::aligned_load);
            } else {
                memcpy_avx2_impl<2, 4>(dst, src, size, V32::unaligned_load);
            }
        }
    }
}

