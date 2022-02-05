#include "../src/memcpy.cpp"
#include <new>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 128) {
        memcpy_amd64::config::allow_ssse3 = data[0];
        memcpy_amd64::config::allow_erms = data[1];
        memcpy_amd64::config::allow_avx512 = data[2];
    }
    auto target = ::operator new(size + 1024, std::align_val_t{1024}); // align destination
    auto dst = static_cast<std::byte *>(target) + (size & 1024);
    auto src = reinterpret_cast<const std::byte *>(data);
    memcpy_amd64::inline_memcpy(dst, src, size);
    if (::memcmp(dst, src, size) != 0) std::abort();
    ::operator delete(target, std::align_val_t{1024});
    return 0;  // Non-zero return values are reserved for future use.
}