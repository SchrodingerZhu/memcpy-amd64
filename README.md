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
Running ./memcpy_bench
Run on (12 X 4400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 1.11, 1.22, 1.33
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system             778389 ns       775833 ns          912 bytes_per_second=125.873k/s
memcpy_mixed_size_custom             673930 ns       671646 ns         1010 bytes_per_second=145.398k/s
memcpy_mixed_size_sse               1084028 ns      1080967 ns          635 bytes_per_second=90.3416k/s
memcpy_less_than_16_system             15.1 ns         15.1 ns     46088454 bytes_per_second=1.54111G/s
memcpy_less_than_16_custom             9.94 ns         9.93 ns     71021508 bytes_per_second=2.34457G/s
memcpy_less_than_16_sse                9.09 ns         9.08 ns     77213828 bytes_per_second=2.56395G/s
memcpy_16_to_128_system                11.9 ns         11.8 ns     59317445 bytes_per_second=1.25908G/s
memcpy_16_to_128_custom                17.1 ns         17.1 ns     41131866 bytes_per_second=892.907M/s
memcpy_16_to_128_sse                   15.0 ns         15.0 ns     46541509 bytes_per_second=1019.4M/s
memcpy_128_to_2048_system              44.3 ns         44.2 ns     15883931 bytes_per_second=345.243M/s
memcpy_128_to_2048_custom              69.8 ns         69.7 ns     10090679 bytes_per_second=218.834M/s
memcpy_128_to_2048_sse                 61.4 ns         61.3 ns     11398463 bytes_per_second=248.771M/s
memcpy_2048_to_786432_system          24083 ns        24045 ns        28826 bytes_per_second=649.823k/s
memcpy_2048_to_786432_custom          24318 ns        24281 ns        28783 bytes_per_second=643.515k/s
memcpy_2048_to_786432_sse             28723 ns        28675 ns        24396 bytes_per_second=544.893k/s
memcpy_786432_to_12582912_system    1295718 ns      1292564 ns          538 bytes_per_second=12.0884k/s
memcpy_786432_to_12582912_custom    1408237 ns      1405234 ns          533 bytes_per_second=11.1191k/s
memcpy_786432_to_12582912_sse       1954672 ns      1949965 ns          359 bytes_per_second=8.01297k/s
memcpy_above_12582912_system        2457234 ns      2452198 ns          284 bytes_per_second=1.59296k/s
memcpy_above_12582912_custom        2442228 ns      2437246 ns          285 bytes_per_second=1.60273k/s
memcpy_above_12582912_sse           3648978 ns      3639843 ns          197 bytes_per_second=1098.95/s
```

- Server (Intel(R) Xeon(R) Silver 4214R CPU @ 2.40GHz, clang 13 + default config)

```
Run on (48 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x24)
  L1 Instruction 32 KiB (x24)
  L2 Unified 1024 KiB (x24)
  L3 Unified 16896 KiB (x2)
Load Average: 3.36, 1.97, 1.82
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system            2487645 ns      2481772 ns          284 bytes_per_second=39.3494k/s
memcpy_mixed_size_custom            2120199 ns      2115221 ns          334 bytes_per_second=46.1684k/s
memcpy_mixed_size_sse               1549846 ns      1546239 ns          449 bytes_per_second=63.1573k/s
memcpy_less_than_16_system             18.2 ns         18.2 ns     39280302 bytes_per_second=1.28149G/s
memcpy_less_than_16_custom             15.2 ns         15.2 ns     49286490 bytes_per_second=1.53141G/s
memcpy_less_than_16_sse                14.1 ns         14.1 ns     48361367 bytes_per_second=1.655G/s
memcpy_16_to_128_system                16.8 ns         16.8 ns     42547202 bytes_per_second=910.822M/s
memcpy_16_to_128_custom                24.2 ns         24.1 ns     31033244 bytes_per_second=631.9M/s
memcpy_16_to_128_sse                   25.4 ns         25.4 ns     28550387 bytes_per_second=601.65M/s
memcpy_128_to_2048_system              75.6 ns         75.4 ns      9340192 bytes_per_second=202.354M/s
memcpy_128_to_2048_custom              89.9 ns         89.7 ns      7860235 bytes_per_second=170.186M/s
memcpy_128_to_2048_sse                 80.8 ns         80.6 ns      8736918 bytes_per_second=189.224M/s
memcpy_2048_to_786432_system          96891 ns        96666 ns         7258 bytes_per_second=161.639k/s
memcpy_2048_to_786432_custom          37472 ns        37384 ns        18919 bytes_per_second=417.955k/s
memcpy_2048_to_786432_sse             57116 ns        56983 ns        12252 bytes_per_second=274.205k/s
memcpy_786432_to_12582912_system    4051207 ns      4041775 ns          171 bytes_per_second=3.86588k/s
memcpy_786432_to_12582912_custom    3675040 ns      3666459 ns          193 bytes_per_second=4.26161k/s
memcpy_786432_to_12582912_sse       2810296 ns      2803757 ns          242 bytes_per_second=5.57288k/s
memcpy_above_12582912_system        7328187 ns      7311130 ns           96 bytes_per_second=547.111/s
memcpy_above_12582912_custom        6445432 ns      6430481 ns          107 bytes_per_second=622.037/s
memcpy_above_12582912_sse           6469311 ns      6454068 ns          125 bytes_per_second=619.764/s
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
Load Average: 2.75, 2.17, 1.92
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system            2478501 ns      2472700 ns          280 bytes_per_second=39.4938k/s
memcpy_mixed_size_custom            1582586 ns      1578889 ns          431 bytes_per_second=61.8512k/s
memcpy_mixed_size_sse               1560704 ns      1557072 ns          438 bytes_per_second=62.7179k/s
memcpy_less_than_16_system             18.1 ns         18.0 ns     38621344 bytes_per_second=1.29039G/s
memcpy_less_than_16_custom             15.8 ns         15.8 ns     46596953 bytes_per_second=1.47489G/s
memcpy_less_than_16_sse                14.4 ns         14.4 ns     47597619 bytes_per_second=1.61793G/s
memcpy_16_to_128_system                16.6 ns         16.5 ns     41805435 bytes_per_second=924.033M/s
memcpy_16_to_128_custom                23.1 ns         23.0 ns     30294705 bytes_per_second=662.797M/s
memcpy_16_to_128_sse                   24.6 ns         24.5 ns     28744569 bytes_per_second=622.231M/s
memcpy_128_to_2048_system              75.1 ns         75.0 ns      9300006 bytes_per_second=203.562M/s
memcpy_128_to_2048_custom              89.5 ns         89.3 ns      7829242 bytes_per_second=170.818M/s
memcpy_128_to_2048_sse                 79.3 ns         79.1 ns      8870527 bytes_per_second=192.972M/s
memcpy_2048_to_786432_system          97397 ns        97170 ns         7199 bytes_per_second=160.801k/s
memcpy_2048_to_786432_custom          55865 ns        55735 ns        12684 bytes_per_second=280.345k/s
memcpy_2048_to_786432_sse             55477 ns        55347 ns        12601 bytes_per_second=282.311k/s
memcpy_786432_to_12582912_system    4114900 ns      4105323 ns          174 bytes_per_second=3.80603k/s
memcpy_786432_to_12582912_custom    2865111 ns      2858444 ns          242 bytes_per_second=5.46626k/s
memcpy_786432_to_12582912_sse       2869664 ns      2862968 ns          239 bytes_per_second=5.45762k/s
memcpy_above_12582912_system        7340310 ns      7323109 ns           95 bytes_per_second=546.216/s
memcpy_above_12582912_custom        5433040 ns      5420341 ns          124 bytes_per_second=737.961/s
memcpy_above_12582912_sse           5462778 ns      5450063 ns          125 bytes_per_second=733.937/s
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
Load Average: 15.46, 15.19, 15.03
-------------------------------------------------------------------------------------------
Benchmark                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------
memcpy_mixed_size_system            1380964 ns      1380891 ns          455 bytes_per_second=70.7198k/s
memcpy_mixed_size_custom            2010818 ns      2010785 ns          360 bytes_per_second=48.5662k/s
memcpy_mixed_size_sse               2646321 ns      2646217 ns          261 bytes_per_second=36.9041k/s
memcpy_less_than_16_system             25.5 ns         25.5 ns     23630916 bytes_per_second=934.065M/s
memcpy_less_than_16_custom             42.1 ns         42.0 ns     21011927 bytes_per_second=567.311M/s
memcpy_less_than_16_sse                35.6 ns         35.6 ns     17458303 bytes_per_second=669.554M/s
memcpy_16_to_128_system                28.2 ns         28.2 ns     30840731 bytes_per_second=541.711M/s
memcpy_16_to_128_custom                58.8 ns         58.8 ns     11239307 bytes_per_second=259.464M/s
memcpy_16_to_128_sse                   27.8 ns         27.8 ns     25224142 bytes_per_second=548.607M/s
memcpy_128_to_2048_system               115 ns          115 ns      6603256 bytes_per_second=133.08M/s
memcpy_128_to_2048_custom               198 ns          198 ns      3620035 bytes_per_second=76.9287M/s
memcpy_128_to_2048_sse                  115 ns          115 ns      6301100 bytes_per_second=132.531M/s
memcpy_2048_to_786432_system          75774 ns        75704 ns         9312 bytes_per_second=206.396k/s
memcpy_2048_to_786432_custom          64244 ns        64242 ns        11440 bytes_per_second=243.22k/s
memcpy_2048_to_786432_sse             84404 ns        84402 ns         8322 bytes_per_second=185.125k/s
memcpy_786432_to_12582912_system    2933028 ns      2932786 ns          232 bytes_per_second=5.3277k/s
memcpy_786432_to_12582912_custom    2775177 ns      2775167 ns          255 bytes_per_second=5.63029k/s
memcpy_786432_to_12582912_sse       4514893 ns      4514365 ns          153 bytes_per_second=3.46117k/s
memcpy_above_12582912_system        5252646 ns      5220127 ns          100 bytes_per_second=766.265/s
memcpy_above_12582912_custom        7735191 ns      7734998 ns           91 bytes_per_second=517.13/s
memcpy_above_12582912_sse           8066591 ns      8062102 ns           87 bytes_per_second=496.149/s
```
