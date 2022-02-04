#include "../include/memcpy-amd64/memcpy.hpp"
#include <cstddef>

#ifndef MEMCPY_AMD64_PREFIX
#define MEMCPY_AMD64_PREFIX custom_
#endif

#define __MEMCPY_AMD64_CONCAT(X, Y) X ## Y
#define MEMCPY_AMD64_CONCAT(X, Y) __MEMCPY_AMD64_CONCAT(X, Y)
#define MEMCPY_AMD64_SYMBOL(X) MEMCPY_AMD64_CONCAT(MEMCPY_AMD64_PREFIX, X)

extern "C" __attribute__((visibility("default"))) void *
MEMCPY_AMD64_SYMBOL(memcpy)(void *__restrict dst, const void *__restrict src, size_t size) {
    return memcpy_amd64::inline_memcpy(dst, src, size);
}

extern "C" __attribute__((visibility("default"))) void
MEMCPY_AMD64_SYMBOL(memcpy_set_erms_threshold)(size_t limit) {
    memcpy_amd64::config::erms_lower_bound = limit;
}

extern "C" __attribute__((visibility("default"))) void
MEMCPY_AMD64_SYMBOL(memcpy_set_nontemporal_threshold)(size_t limit) {
    memcpy_amd64::config::non_temporal_lower_bound = limit;
}

extern "C" __attribute__((visibility("default"))) void
MEMCPY_AMD64_SYMBOL(memcpy_set_avx512)(bool status) {
    memcpy_amd64::config::allow_avx512 = status;
}

extern "C" __attribute__((visibility("default"))) size_t
MEMCPY_AMD64_SYMBOL(memcpy_get_erms_threshold)() {
    return memcpy_amd64::config::erms_lower_bound;
}

extern "C" __attribute__((visibility("default"))) size_t
MEMCPY_AMD64_SYMBOL(memcpy_get_nontemporal_threshold)() {
    return memcpy_amd64::config::non_temporal_lower_bound;
}

extern "C" __attribute__((visibility("default"))) bool
MEMCPY_AMD64_SYMBOL(memcpy_get_avx512)() {
    return memcpy_amd64::config::allow_avx512;
}

namespace memcpy_amd64::config {
    size_t erms_lower_bound = 2048;
    size_t non_temporal_lower_bound = 0xc0000;
    bool allow_avx512 = true;
}