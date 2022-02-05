#include "../include/memcpy-amd64/memcpy.hpp"
#include <new>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 256) {
        auto target = ::operator new(size, std::align_val_t{16}); // align destination
        auto dst = static_cast<std::byte *>(target);
        auto src = reinterpret_cast<const std::byte *>(data);
        auto cursor = size;
        memcpy_amd64::detail::memcpy_ssse3_mux(dst, src, cursor);
        __builtin_memcpy(dst, src, cursor);
        if (::memcmp(data, target, size) != 0) std::abort();
        ::operator delete(target, std::align_val_t{16});
    }
    return 0;  // Non-zero return values are reserved for future use.
}