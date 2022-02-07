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

#ifdef __clang__
#define MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY __builtin_memcpy_inline
#define MEMCPY_AMD64_UNROLL_FULLY _Pragma("clang loop unroll(full)")
#else
#define MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY __builtin_memcpy
#define MEMCPY_AMD64_UNROLL_FULLY _Pragma("GCC unroll 65534")
#endif

namespace memcpy_amd64 {

    namespace detail {

        __attribute__((always_inline)) static inline
        void cpuid(int32_t (&out)[4], int32_t eax, int32_t ecx) {
            __cpuid_count(eax, ecx, out[0], out[1], out[2], out[3]);
        }

        __attribute__((always_inline)) static inline
        bool rep_movsb(void *__restrict dst, const void *__restrict src, size_t size) {
            asm volatile("rep movsb"
            : "+D"(dst), "+S"(src), "+c"(size)
            :
            : "memory");
            return dst;
        }

        __attribute__((target("avx2"))) static inline void memcpy_avx2_page2(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size);

        __attribute__((target("avx2"))) static inline void memcpy_avx2_page4(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size);

        __attribute__((target("avx512vl,avx512f,ssse3"))) static inline void memcpy_evex_page2(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size);

        __attribute__((target("avx512vl,avx512f,ssse3"))) static inline void memcpy_evex_page4(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size);

        __attribute__((always_inline)) static inline void memcpy_sse_loop(std::byte *__restrict &dst,
                                                                          const std::byte *__restrict &src,
                                                                          size_t &size) {
            __m128i c0, c1, c2, c3, c4, c5, c6, c7;

            while (size >= 128) {
                const auto *source = reinterpret_cast<const __m128i *>(src);
                auto *dest = __builtin_assume_aligned(reinterpret_cast<const __m128i *>(dst), 16);

                c0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source) + 0);
                c1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source) + 1);
                c2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source) + 2);
                c3 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source) + 3);
                c4 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source) + 4);
                c5 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source) + 5);
                c6 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source) + 6);
                c7 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(source) + 7);
                src += 128;
                _mm_store_si128((reinterpret_cast<__m128i *>(dest) + 0), c0);
                _mm_store_si128((reinterpret_cast<__m128i *>(dest) + 1), c1);
                _mm_store_si128((reinterpret_cast<__m128i *>(dest) + 2), c2);
                _mm_store_si128((reinterpret_cast<__m128i *>(dest) + 3), c3);
                _mm_store_si128((reinterpret_cast<__m128i *>(dest) + 4), c4);
                _mm_store_si128((reinterpret_cast<__m128i *>(dest) + 5), c5);
                _mm_store_si128((reinterpret_cast<__m128i *>(dest) + 6), c6);
                _mm_store_si128((reinterpret_cast<__m128i *>(dest) + 7), c7);
                dst += 128;
                size -= 128;
            }
        }

        template<uint8_t delta>
        __attribute__((target("ssse3"))) static inline void memcpy_ssse3_loop(std::byte *__restrict &dst,
                                                                              const std::byte *__restrict &src,
                                                                              size_t &size) {
            src -= delta;
            size += delta;

            __m128i c0, c1, c2, c3, c4, c5, c6, c7, c8;
            while (size >= 144) {
                const auto *source = __builtin_assume_aligned(reinterpret_cast<const __m128i *>(src), 16);
                auto *dest = __builtin_assume_aligned(reinterpret_cast<const __m128i *>(dst), 16);

                __builtin_prefetch(src + 8 * 16);

                c0 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 0);
                c1 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 1);
                c2 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 2);
                c3 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 3);
                c4 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 4);
                c5 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 5);
                c6 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 6);
                c7 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 7);
                c8 = _mm_load_si128(reinterpret_cast<const __m128i *>(source) + 8);

                src += 128;

                if constexpr(delta != 0) {
                    c0 = _mm_alignr_epi8(c1, c0, delta);
                    c1 = _mm_alignr_epi8(c2, c1, delta);
                    c2 = _mm_alignr_epi8(c3, c2, delta);
                    c3 = _mm_alignr_epi8(c4, c3, delta);
                    c4 = _mm_alignr_epi8(c5, c4, delta);
                    c5 = _mm_alignr_epi8(c6, c5, delta);
                    c6 = _mm_alignr_epi8(c7, c6, delta);
                    c7 = _mm_alignr_epi8(c8, c7, delta);
                }

                _mm_stream_si128((reinterpret_cast<__m128i *>(dest) + 0), c0);
                _mm_stream_si128((reinterpret_cast<__m128i *>(dest) + 1), c1);
                _mm_stream_si128((reinterpret_cast<__m128i *>(dest) + 2), c2);
                _mm_stream_si128((reinterpret_cast<__m128i *>(dest) + 3), c3);
                _mm_stream_si128((reinterpret_cast<__m128i *>(dest) + 4), c4);
                _mm_stream_si128((reinterpret_cast<__m128i *>(dest) + 5), c5);
                _mm_stream_si128((reinterpret_cast<__m128i *>(dest) + 6), c6);
                _mm_stream_si128((reinterpret_cast<__m128i *>(dest) + 7), c7);
                dst += 128;
                size -= 128;
            }
            dst -= delta;
        }

        __attribute__((target("ssse3"))) static inline void memcpy_ssse3_mux(std::byte *__restrict &dst,
                                                                             const std::byte *__restrict &src,
                                                                             size_t &size) {
            MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, 16);
            dst += 16;
            src += 16;
            size -= 16;

            auto delta = reinterpret_cast<uintptr_t>(src) % 16;

            switch (delta) {
                case 0:
                    memcpy_ssse3_loop<0>(dst, src, size);
                    break;
                case 1:
                    memcpy_ssse3_loop<1>(dst, src, size);
                    break;
                case 2:
                    memcpy_ssse3_loop<2>(dst, src, size);
                    break;
                case 3:
                    memcpy_ssse3_loop<3>(dst, src, size);
                    break;
                case 4:
                    memcpy_ssse3_loop<4>(dst, src, size);
                    break;
                case 5:
                    memcpy_ssse3_loop<5>(dst, src, size);
                    break;
                case 6:
                    memcpy_ssse3_loop<6>(dst, src, size);
                    break;
                case 7:
                    memcpy_ssse3_loop<7>(dst, src, size);
                    break;
                case 8:
                    memcpy_ssse3_loop<8>(dst, src, size);
                    break;
                case 9:
                    memcpy_ssse3_loop<9>(dst, src, size);
                    break;
                case 10:
                    memcpy_ssse3_loop<10>(dst, src, size);
                    break;
                case 11:
                    memcpy_ssse3_loop<11>(dst, src, size);
                    break;
                case 12:
                    memcpy_ssse3_loop<12>(dst, src, size);
                    break;
                case 13:
                    memcpy_ssse3_loop<13>(dst, src, size);
                    break;
                case 14:
                    memcpy_ssse3_loop<14>(dst, src, size);
                    break;
                default:
                    memcpy_ssse3_loop<15>(dst, src, size);
                    break;
            }
        }
    }

    namespace config {
        static inline constexpr size_t page_size = 0x1000;
        extern size_t erms_lower_bound;
        extern size_t non_temporal_lower_bound;
        extern bool allow_avx512;
        extern bool allow_erms;
        extern bool allow_ssse3;
    };

    namespace vectorize {
        struct VEX32 {
            using vector = __m256i;

            struct Container {
                vector data;
            };

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

        struct EVEX64 {
            using vector = __m512i;

            struct Container {
                vector data;
            };

            __attribute__((always_inline, target("avx512vl,avx512f,ssse3"))) static inline vector
            aligned_load(const void *address) {
                return _mm512_load_si512(static_cast<const vector *>(address));
            }

            __attribute__((always_inline, target("avx512vl,avx512f,ssse3"))) static inline vector
            unaligned_load(const void *address) {
                return _mm512_loadu_si512(static_cast<const vector *>(address));
            }

            __attribute__((always_inline, target("avx512vl,avx512f,ssse3"))) static inline vector
            nt_load(const void *address) {
                return _mm512_stream_load_si512(static_cast<vector *>(const_cast<void *>(address)));
            }

            __attribute__((always_inline, target("avx512vl,avx512f,ssse3"))) static inline void
            aligned_store(void *address, const vector &val) {
                return _mm512_store_si512(static_cast<vector *>(address), val);
            }

            __attribute__((always_inline, target("avx512vl,avx512f,ssse3"))) static inline void
            unaligned_store(void *address, const vector &val) {
                return _mm512_storeu_si512(static_cast<vector *>(address), val);
            }

            __attribute__((always_inline, target("avx512vl,avx512f,ssse3"))) static inline void
            nt_store(void *address, const vector &val) {
                return _mm512_stream_si512(static_cast<vector *>(address), val);
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
        if (__builtin_expect(size <= 16, 1)) {
            if (size >= 8) {
                /// Chunks of 8..16 bytes.
                MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst + size - 8, src + size - 8, 8);
                MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, 8);
            } else if (size >= 4) {
                /// Chunks of 4..7 bytes.
                MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst + size - 4, src + size - 4, 4);
                MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, 4);
            } else if (size >= 2) {
                /// Chunks of 2..3 bytes.
                MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst + size - 2, src + size - 2, 2);
                MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, 2);
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
                MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst + size - 16, src + size - 16, 16);

                /// Then we will copy every 16 bytes from the beginning in a loop.
                /// The last loop iteration will possibly overwrite some part of already copied last 16 bytes.
                /// This is Ok, similar to the code for small sizes above.
                while (size > 16) {
                    MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, 16);
                    dst += 16;
                    src += 16;
                    size -= 16;
                }
            } else {
                if (__builtin_expect(size < config::erms_lower_bound, 1)) {
                    size_t padding = (-reinterpret_cast<size_t>(dst)) & 15;

                    // avoid branch
                    MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, 16);
                    dst += padding;
                    src += padding;
                    size -= padding;

                    /// Aligned unrolled copy. We will use half of available SSE registers.
                    /// It's not possible to have both src and dst aligned.
                    /// So, we will use aligned stores and unaligned loads.
                    detail::memcpy_sse_loop(dst, src, size);
                } else {
                    auto body = [&]() __attribute__((noinline)) {
                        int out[4];
                        detail::cpuid(out, 0x00000007, 0);
                        if (size >= config::non_temporal_lower_bound) {
                            if ((out[1] & (1 << 31)) != 0 && config::allow_avx512) {
                                if (size >= 16 * config::non_temporal_lower_bound) {
                                    detail::memcpy_evex_page4(dst, src, size);
                                } else {
                                    detail::memcpy_evex_page2(dst, src, size);
                                }
                            } else if ((out[1] & (1 << 5)) != 0) {
                                if (size >= 16 * config::non_temporal_lower_bound) {
                                    detail::memcpy_avx2_page4(dst, src, size);
                                } else {
                                    detail::memcpy_avx2_page2(dst, src, size);
                                }
                            }
                        }
                        if (size >= config::erms_lower_bound && (out[1] & (1 << 9)) != 0 && config::allow_erms) {
                            detail::rep_movsb(dst, src, size);
                            size = 0;
                        }
                        if (size > 128) {
                            size_t padding = (-reinterpret_cast<size_t>(dst)) & 15;
                            MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, 16);
                            dst += padding;
                            src += padding;
                            size -= padding;
                            detail::cpuid(out, 0x00000001, 0);
                            if (config::allow_ssse3 && (out[2] & (1 << 9)) != 0) {
                                detail::memcpy_ssse3_mux(dst, src, size);
                            } else {
                                detail::memcpy_sse_loop(dst, src, size);
                            }
                        }
                    };
                    body();
                }
                goto tail;
            }
        }
        return ret;
    }


    namespace detail {
        template<typename VecTrait, size_t page_num, size_t vec_num, typename Load>
        __attribute__((always_inline, target("avx2"))) static inline void memcpy_avx2_impl(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size,
                Load load) {
            using namespace vectorize;
            using vector = typename VecTrait::vector;
            using Container = typename VecTrait::Container;
            constexpr size_t stride_size = vec_num * sizeof(vector);
            std::array<Container, vec_num * page_num> storage{};

            while (size >= page_num * config::page_size) {
                for (size_t i = 0; i < config::page_size / stride_size; ++i) {
                    auto target = static_cast<std::byte *>(__builtin_assume_aligned(dst, alignof(vector)));
                    MEMCPY_AMD64_UNROLL_FULLY
                    for (size_t p = 0; p < page_num; ++p) {
                        __builtin_prefetch(src + config::page_size * p + stride_size);
                    };

                    MEMCPY_AMD64_UNROLL_FULLY
                    for (size_t p = 0; p < page_num; ++p) {
                        MEMCPY_AMD64_UNROLL_FULLY
                        for (size_t v = 0; v < vec_num; ++v) {
                            storage[p * 4 + v].data = load(src + config::page_size * p + sizeof(vector) * v);
                        };
                    };

                    MEMCPY_AMD64_UNROLL_FULLY
                    for (size_t p = 0; p < page_num; ++p) {
                        MEMCPY_AMD64_UNROLL_FULLY
                        for (size_t v = 0; v < vec_num; ++v) {
                            VecTrait::nt_store(target + config::page_size * p + sizeof(vector) * v,
                                               storage[p * 4 + v].data);
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

        template<typename VecTrait, size_t page_num, size_t vec_num, typename Load>
        __attribute__((always_inline, target("avx512vl,avx512f,ssse3"))) static inline void memcpy_evex_impl(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size,
                Load load) {
            using namespace vectorize;
            using vector = typename VecTrait::vector;
            using Container = typename VecTrait::Container;
            constexpr size_t stride_size = vec_num * sizeof(vector);
            std::array<Container, vec_num * page_num> storage{};

            while (size >= page_num * config::page_size) {
                for (size_t i = 0; i < config::page_size / stride_size; ++i) {
                    auto target = static_cast<std::byte *>(__builtin_assume_aligned(dst, alignof(vector)));
                    MEMCPY_AMD64_UNROLL_FULLY
                    for (size_t p = 0; p < page_num; ++p) {
                        __builtin_prefetch(src + config::page_size * p + stride_size);
                    };

                    MEMCPY_AMD64_UNROLL_FULLY
                    for (size_t p = 0; p < page_num; ++p) {
                        MEMCPY_AMD64_UNROLL_FULLY
                        for (size_t v = 0; v < vec_num; ++v) {
                            storage[p * vec_num + v].data = load(src + config::page_size * p + sizeof(vector) * v);
                        };
                    };

                    MEMCPY_AMD64_UNROLL_FULLY
                    for (size_t p = 0; p < page_num; ++p) {
                        MEMCPY_AMD64_UNROLL_FULLY
                        for (size_t v = 0; v < vec_num; ++v) {
                            VecTrait::nt_store(target + config::page_size * p + sizeof(vector) * v,
                                               storage[p * vec_num + v].data);
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

        __attribute__((target("avx2"))) static inline void memcpy_avx2_page4(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size) {
            using namespace vectorize;
            using vector = VEX32::vector;
            auto dst_padding = (-reinterpret_cast<uintptr_t>(dst)) & (sizeof(vector) - 1);
            auto diff =
                    (reinterpret_cast<uintptr_t>(dst) ^ reinterpret_cast<uintptr_t>(src)) & (sizeof(vector) - 1);

            MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, sizeof(vector));
            dst += dst_padding;
            src += dst_padding;
            size -= dst_padding;

            if (__builtin_expect(diff == 0, 0)) {
                memcpy_avx2_impl<VEX32, 4, 4>(dst, src, size, VEX32::aligned_load);
            } else {
                memcpy_avx2_impl<VEX32, 4, 4>(dst, src, size, VEX32::unaligned_load);
            }
        }

        __attribute__((target("avx2"))) static inline void memcpy_avx2_page2(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size) {
            using namespace vectorize;
            using vector = VEX32::vector;
            auto dst_padding = (-reinterpret_cast<uintptr_t>(dst)) & (sizeof(vector) - 1);
            auto diff =
                    (reinterpret_cast<uintptr_t>(dst) ^ reinterpret_cast<uintptr_t>(src)) & (sizeof(vector) - 1);

            MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, sizeof(vector));
            dst += dst_padding;
            src += dst_padding;
            size -= dst_padding;


            if (__builtin_expect(diff == 0, 0)) {
                memcpy_avx2_impl<VEX32, 2, 4>(dst, src, size, VEX32::aligned_load);
            } else {
                memcpy_avx2_impl<VEX32, 2, 4>(dst, src, size, VEX32::unaligned_load);
            }
        }

        __attribute__((target("avx512vl,avx512f,ssse3"))) static inline void memcpy_evex_page4(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size) {
            using namespace vectorize;
            using vector = EVEX64::vector;
            auto dst_padding = (-reinterpret_cast<uintptr_t>(dst)) & (sizeof(vector) - 1);
            auto diff =
                    (reinterpret_cast<uintptr_t>(dst) ^ reinterpret_cast<uintptr_t>(src)) & (sizeof(vector) - 1);

            MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, sizeof(vector));
            dst += dst_padding;
            src += dst_padding;
            size -= dst_padding;

            if (__builtin_expect(diff == 0, 0)) {
                memcpy_evex_impl<EVEX64, 4, 4>(dst, src, size, EVEX64::aligned_load);
            } else {
                memcpy_evex_impl<EVEX64, 4, 4>(dst, src, size, EVEX64::unaligned_load);
            }
            detail::memcpy_sse_loop(dst, src, size);
        }

        __attribute__((target("avx512vl,avx512f,ssse3"))) static inline void memcpy_evex_page2(
                std::byte *__restrict &dst,
                const std::byte *__restrict &src,
                size_t &size) {
            using namespace vectorize;
            using vector = EVEX64::vector;
            auto dst_padding = (-reinterpret_cast<uintptr_t>(dst)) & (sizeof(vector) - 1);
            auto diff =
                    (reinterpret_cast<uintptr_t>(dst) ^ reinterpret_cast<uintptr_t>(src)) & (sizeof(vector) - 1);

            MEMCPY_AMD64_COMPILER_BUILTIN_MEMCPY(dst, src, sizeof(vector));
            dst += dst_padding;
            src += dst_padding;
            size -= dst_padding;

            if (__builtin_expect(diff == 0, 0)) {
                memcpy_evex_impl<EVEX64, 2, 4>(dst, src, size, EVEX64::aligned_load);
            } else {
                memcpy_evex_impl<EVEX64, 2, 4>(dst, src, size, EVEX64::unaligned_load);
            }
            detail::memcpy_sse_loop(dst, src, size);
        }
    }
}

