# memcpy-amd64

A memcpy library for amd64 platforms.

## Parameters

- `void memcpy_<set/get>_erms_threshold(size_t)` (default 2048): threshold to use `rep movsb`
- `void memcpy_<set/get>_nontemporal_threshold(size_t)` (default 0xc0000): threshold to use multi-way streaming store
- `void memcpy_<set/get>_avx512(bool)` (default true): whether avx512 is allowed.

## Some Results

**Before the benchmarks**: There is **NO** silver bullet in memcpy. For daily use, you should use libc's memcpy (or maybe use `__builtin_memcpy_inline` for small small constant size objects), unless you know exactly the workload pattern. BTW, please notice that memcpy performance varies a lot with different micro-architectures. Premature optimization is the root of all evil. As you can see below, even on some modern CPUs, spartan SSE2 implementation ranks the first; so do run some tests before customize your own memcpy.

- Laptop (Intel(R) Xeon(R) E-2176M  CPU @ 2.70GHz, clang 13 + default config)

```
2022-02-04T21:07:16+08:00
Running ./memcpy_bench
Run on (12 X 4400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 1.58, 1.65, 1.55
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system             770784 ns       768236 ns          901 bytes_per_second=16.2192G/s
memcpy_mixed_size_custom             667133 ns       664847 ns         1058 bytes_per_second=18.7414G/s
memcpy_mixed_size_sse               1106996 ns      1082144 ns          636 bytes_per_second=11.5143G/s
memcpy_less_than_16_system             15.1 ns         15.1 ns     46053045 bytes_per_second=1.47915G/s
memcpy_less_than_16_custom             10.7 ns         10.7 ns     65778395 bytes_per_second=2.09362G/s
memcpy_less_than_16_sse                10.3 ns         10.3 ns     68010901 bytes_per_second=2.17725G/s
memcpy_16_to_128_system                11.8 ns         11.8 ns     58327706 bytes_per_second=21.7707G/s
memcpy_16_to_128_custom                17.4 ns         17.3 ns     40524837 bytes_per_second=14.7735G/s
memcpy_16_to_128_sse                   17.5 ns         17.5 ns     40203942 bytes_per_second=14.6449G/s
memcpy_128_to_2048_system              46.5 ns         46.4 ns     15156748 bytes_per_second=69.2751G/s
memcpy_128_to_2048_custom              70.7 ns         70.6 ns      9141354 bytes_per_second=45.5741G/s
memcpy_128_to_2048_sse                 62.7 ns         62.6 ns     10097050 bytes_per_second=51.3685G/s
memcpy_2048_to_786432_system          24208 ns        24175 ns        29053 bytes_per_second=30.9185G/s
memcpy_2048_to_786432_custom          24471 ns        24439 ns        28844 bytes_per_second=30.5842G/s
memcpy_2048_to_786432_sse             29253 ns        29205 ns        24052 bytes_per_second=25.5937G/s
memcpy_786432_to_12582912_system    1321804 ns      1318805 ns          522 bytes_per_second=14.995G/s
memcpy_786432_to_12582912_custom    1392282 ns      1389181 ns          500 bytes_per_second=14.2353G/s
memcpy_786432_to_12582912_sse       1918728 ns      1914238 ns          363 bytes_per_second=10.3307G/s
memcpy_above_12582912_system        2576338 ns      2571399 ns          266 bytes_per_second=13.6722G/s
memcpy_above_12582912_custom        2543518 ns      2531713 ns          260 bytes_per_second=13.8865G/s
memcpy_above_12582912_sse           3685330 ns      3677535 ns          192 bytes_per_second=9.55986G/s
```

- Server (Intel(R) Xeon(R) Silver 4214R CPU @ 2.40GHz, clang 13 + default config)

```
Running ./memcpy_bench
Run on (48 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x24)
  L1 Instruction 32 KiB (x24)
  L2 Unified 1024 KiB (x24)
  L3 Unified 16896 KiB (x2)
Load Average: 1.54, 1.88, 1.83
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system            2481572 ns      2475797 ns          285 bytes_per_second=5.0328G/s
memcpy_mixed_size_custom            2131137 ns      2126152 ns          332 bytes_per_second=5.86044G/s
memcpy_mixed_size_sse               1749279 ns      1745196 ns          371 bytes_per_second=7.1397G/s
memcpy_less_than_16_system             19.4 ns         19.3 ns     36184114 bytes_per_second=1.15608G/s
memcpy_less_than_16_custom             15.1 ns         15.1 ns     46251725 bytes_per_second=1.48447G/s
memcpy_less_than_16_sse                14.7 ns         14.7 ns     48736368 bytes_per_second=1.52087G/s
memcpy_16_to_128_system                17.1 ns         17.1 ns     40678331 bytes_per_second=14.9946G/s
memcpy_16_to_128_custom                23.3 ns         23.2 ns     29825474 bytes_per_second=11.0159G/s
memcpy_16_to_128_sse                   21.4 ns         21.4 ns     32414358 bytes_per_second=11.9729G/s
memcpy_128_to_2048_system              78.7 ns         78.5 ns      8881922 bytes_per_second=40.9675G/s
memcpy_128_to_2048_custom              94.2 ns         94.0 ns      7473891 bytes_per_second=34.2221G/s
memcpy_128_to_2048_sse                 81.4 ns         81.2 ns      8554158 bytes_per_second=39.6265G/s
memcpy_2048_to_786432_system          98399 ns        98168 ns         7223 bytes_per_second=7.61409G/s
memcpy_2048_to_786432_custom          33959 ns        33879 ns        20812 bytes_per_second=22.0622G/s
memcpy_2048_to_786432_sse             57137 ns        57003 ns        12251 bytes_per_second=13.1127G/s
memcpy_786432_to_12582912_system    4064951 ns      4055490 ns          173 bytes_per_second=4.87621G/s
memcpy_786432_to_12582912_custom    3728751 ns      3720031 ns          184 bytes_per_second=5.31593G/s
memcpy_786432_to_12582912_sse       2974453 ns      2967503 ns          241 bytes_per_second=6.664G/s
memcpy_above_12582912_system        7400427 ns      7383127 ns           92 bytes_per_second=4.76177G/s
memcpy_above_12582912_custom        6704891 ns      6689249 ns          106 bytes_per_second=5.25571G/s
memcpy_above_12582912_sse           5582536 ns      5569464 ns          124 bytes_per_second=6.31241G/s
```

- Server (Intel(R) Xeon(R) Silver 4214R CPU @ 2.40GHz, clang 13 + ERMS_THRESHOLD=-1 + NONTERMPORAL_THRESHOLD=-1)
```
Running ./memcpy_bench
Run on (48 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x24)
  L1 Instruction 32 KiB (x24)
  L2 Unified 1024 KiB (x24)
  L3 Unified 16896 KiB (x2)
Load Average: 2.06, 1.99, 1.88
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system            2511528 ns      2505614 ns          281 bytes_per_second=4.9729G/s
memcpy_mixed_size_custom            1638262 ns      1634431 ns          419 bytes_per_second=7.62356G/s
memcpy_mixed_size_sse               1634938 ns      1631117 ns          420 bytes_per_second=7.63905G/s
memcpy_less_than_16_system             19.4 ns         19.4 ns     36241776 bytes_per_second=1.15383G/s
memcpy_less_than_16_custom             15.1 ns         15.0 ns     46403428 bytes_per_second=1.48637G/s
memcpy_less_than_16_sse                14.6 ns         14.5 ns     47782487 bytes_per_second=1.53949G/s
memcpy_16_to_128_system                17.1 ns         17.0 ns     40862325 bytes_per_second=15.0456G/s
memcpy_16_to_128_custom                23.6 ns         23.5 ns     29244932 bytes_per_second=10.8864G/s
memcpy_16_to_128_sse                   21.7 ns         21.6 ns     32175307 bytes_per_second=11.8457G/s
memcpy_128_to_2048_system              78.6 ns         78.4 ns      8717835 bytes_per_second=41.0267G/s
memcpy_128_to_2048_custom              95.6 ns         95.4 ns      7442825 bytes_per_second=33.7334G/s
memcpy_128_to_2048_sse                 84.0 ns         83.8 ns      8356702 bytes_per_second=38.384G/s
memcpy_2048_to_786432_system         100457 ns       100222 ns         7031 bytes_per_second=7.45803G/s
memcpy_2048_to_786432_custom          56726 ns        56594 ns        12408 bytes_per_second=13.2074G/s
memcpy_2048_to_786432_sse             56614 ns        56481 ns        12341 bytes_per_second=13.2337G/s
memcpy_786432_to_12582912_system    4135227 ns      4125498 ns          170 bytes_per_second=4.79347G/s
memcpy_786432_to_12582912_custom    2952794 ns      2945873 ns          237 bytes_per_second=6.71293G/s
memcpy_786432_to_12582912_sse       2867056 ns      2860383 ns          242 bytes_per_second=6.91356G/s
memcpy_above_12582912_system        7370900 ns      7353578 ns           94 bytes_per_second=4.7809G/s
memcpy_above_12582912_custom        5494519 ns      5481667 ns          124 bytes_per_second=6.41351G/s
memcpy_above_12582912_sse           5507822 ns      5494972 ns          123 bytes_per_second=6.39798G/s
```

- Server (Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz + default config)

```
Running ./memcpy_bench
Run on (40 X 2399.94 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x20)
  L1 Instruction 32 KiB (x20)
  L2 Unified 256 KiB (x20)
  L3 Unified 25600 KiB (x2)
Load Average: 13.77, 14.83, 15.09
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system            1371039 ns      1370929 ns          467 bytes_per_second=9.08886G/s
memcpy_mixed_size_custom            2261065 ns      2260929 ns          308 bytes_per_second=5.51109G/s
memcpy_mixed_size_sse               2932374 ns      2932282 ns          241 bytes_per_second=4.24931G/s
memcpy_less_than_16_system             26.5 ns         26.5 ns     27635914 bytes_per_second=865.143M/s
memcpy_less_than_16_custom             27.9 ns         27.9 ns     23564349 bytes_per_second=819.427M/s
memcpy_less_than_16_sse                27.1 ns         27.1 ns     26093019 bytes_per_second=844.046M/s
memcpy_16_to_128_system                24.0 ns         24.0 ns     29689172 bytes_per_second=10.6928G/s
memcpy_16_to_128_custom                62.8 ns         62.8 ns     11186669 bytes_per_second=4.08097G/s
memcpy_16_to_128_sse                   30.6 ns         30.4 ns     23112386 bytes_per_second=8.41854G/s
memcpy_128_to_2048_system               105 ns          105 ns      5925336 bytes_per_second=30.5266G/s
memcpy_128_to_2048_custom               204 ns          204 ns      3550763 bytes_per_second=15.7591G/s
memcpy_128_to_2048_sse                  152 ns          152 ns      6162336 bytes_per_second=21.1647G/s
memcpy_2048_to_786432_system          79217 ns        79215 ns         9468 bytes_per_second=9.43576G/s
memcpy_2048_to_786432_custom          61352 ns        61352 ns        11613 bytes_per_second=12.183G/s
memcpy_2048_to_786432_sse             87446 ns        87318 ns         8344 bytes_per_second=8.56017G/s
memcpy_786432_to_12582912_system    3003465 ns      3003413 ns          237 bytes_per_second=6.58432G/s
memcpy_786432_to_12582912_custom    2702430 ns      2702420 ns          223 bytes_per_second=7.31767G/s
memcpy_786432_to_12582912_sse       4605743 ns      4605610 ns          155 bytes_per_second=4.29377G/s
memcpy_above_12582912_system        5370893 ns      5369575 ns          116 bytes_per_second=6.5474G/s
memcpy_above_12582912_custom        7936271 ns      7935497 ns           88 bytes_per_second=4.43031G/s
memcpy_above_12582912_sse           8206285 ns      8206107 ns           86 bytes_per_second=4.28421G/s
```

- Server (Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz + ALLOW_ERMS=0 + MEMCPY_NONTERMPORAL_THRESHOLD=-1)

```
Running ./memcpy_bench
Run on (40 X 2399.94 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x20)
  L1 Instruction 32 KiB (x20)
  L2 Unified 256 KiB (x20)
  L3 Unified 25600 KiB (x2)
Load Average: 16.11, 16.31, 15.31
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system            1571859 ns      1571612 ns          500 bytes_per_second=7.92828G/s
memcpy_mixed_size_custom            1128255 ns      1128251 ns          573 bytes_per_second=11.0438G/s
memcpy_mixed_size_sse               2567925 ns      2567886 ns          272 bytes_per_second=4.85231G/s
memcpy_less_than_16_system             24.0 ns         24.0 ns     29246726 bytes_per_second=955.222M/s
memcpy_less_than_16_custom             28.6 ns         28.6 ns     25047389 bytes_per_second=799.29M/s
memcpy_less_than_16_sse                23.7 ns         23.7 ns     22610847 bytes_per_second=965.47M/s
memcpy_16_to_128_system                23.5 ns         23.5 ns     27943523 bytes_per_second=10.9158G/s
memcpy_16_to_128_custom                60.4 ns         60.4 ns      9043475 bytes_per_second=4.23706G/s
memcpy_16_to_128_sse                   34.6 ns         34.6 ns     24649936 bytes_per_second=7.40853G/s
memcpy_128_to_2048_system               108 ns          107 ns      6288841 bytes_per_second=30.0187G/s
memcpy_128_to_2048_custom               191 ns          191 ns      3641932 bytes_per_second=16.8819G/s
memcpy_128_to_2048_sse                  112 ns          112 ns      6415427 bytes_per_second=28.7896G/s
memcpy_2048_to_786432_system          81344 ns        81073 ns         9204 bytes_per_second=9.21951G/s
memcpy_2048_to_786432_custom          66610 ns        66606 ns        10640 bytes_per_second=11.222G/s
memcpy_2048_to_786432_sse             86072 ns        86070 ns         8327 bytes_per_second=8.68427G/s
memcpy_786432_to_12582912_system    2958925 ns      2958875 ns          231 bytes_per_second=6.68343G/s
memcpy_786432_to_12582912_custom    2199050 ns      2198987 ns          336 bytes_per_second=8.99297G/s
memcpy_786432_to_12582912_sse       4555411 ns      4555393 ns          151 bytes_per_second=4.3411G/s
memcpy_above_12582912_system        5195001 ns      5194925 ns          136 bytes_per_second=6.76751G/s
memcpy_above_12582912_custom        4581617 ns      4548292 ns          155 bytes_per_second=7.72966G/s
memcpy_above_12582912_sse           8489892 ns      8489583 ns           87 bytes_per_second=4.14116G/s
```
