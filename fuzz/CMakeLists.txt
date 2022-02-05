add_executable(fuzz_sse_loop fuzz_sse_loop.cpp)
target_link_options(fuzz_sse_loop PRIVATE -fsanitize=fuzzer,address )
target_compile_options(fuzz_sse_loop PRIVATE -fsanitize=fuzzer,address)

add_executable(fuzz_ssse3_loop fuzz_ssse3_loop.cpp)
target_link_options(fuzz_ssse3_loop PRIVATE -fsanitize=fuzzer,address )
target_compile_options(fuzz_ssse3_loop PRIVATE -fsanitize=fuzzer,address)

add_executable(fuzz_memcpy fuzz_memcpy.cpp)
target_link_options(fuzz_memcpy PRIVATE -fsanitize=fuzzer,address )
target_compile_options(fuzz_memcpy PRIVATE -fsanitize=fuzzer,address)