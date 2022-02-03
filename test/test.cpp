//
// Created by schrodinger on 2/3/22.
//

#include <gtest/gtest.h>
#define MEMCPY_AMD64_PREFIX custom_
#include "../src/memcpy.cpp"
#include <vector>
#include <random>

std::vector<char> gen_data(size_t size) {
    std::random_device rdev;
    std::default_random_engine eng(rdev());
    std::uniform_int_distribution<char> dist;
    std::vector<char> data(size);
    for (auto & i : data) {
        i = dist(eng);
    }
    return data;
}

TEST(MemCopy, Small) {
    for (auto i : {0, 1, 2, 3, 7, 9, 16, 32, 33, 57, 127, 122}) {
        auto data = gen_data(i);
        std::vector<char> data2(i);
        custom_memcpy(data2.data(), data.data(), data.size());
        EXPECT_EQ(data, data2) << "size : " << i;
    }
}

TEST(MemCopy, Range128To2048) {
    for (auto i : {128, 129, 130, 256, 512, 533, 1020, 1024, 2047}) {
        auto data = gen_data(i);
        std::vector<char> data2(i);
        custom_memcpy(data2.data(), data.data(), data.size());
        EXPECT_EQ(data, data2) << "size : " << i;
    }
}

TEST(MemCopy, Range2048To786432) {
    for (auto i : {2048, 2049, 2050, 4096, 4097, 786431, 70000}) {
        auto data = gen_data(i);
        std::vector<char> data2(i);
        custom_memcpy(data2.data(), data.data(), data.size());
        EXPECT_EQ(data, data2) << "size : " << i;
    }
}

TEST(MemCopy, Range786432To12582912Unaligned) {
    for (auto i : {786432, 786433, 786435, 1786432, 999999, 12582911, 12582910, 12582909}) {
        auto data = gen_data(i);
        std::vector<char> data2(i);
        auto mem1 = static_cast<std::byte*>(::operator new (data.size() + 512, std::align_val_t {512}));
        auto mem2 = static_cast<std::byte*>(::operator new (data.size() + 512, std::align_val_t {512}));
        mem1 += 13;
        custom_memcpy(mem1, data.data(), data.size());
        custom_memcpy(mem2, mem1, data.size());
        custom_memcpy(data2.data(), mem2, data.size());
        EXPECT_EQ(data, data2) << "size : " << i;
    }
}

TEST(MemCopy, Range786432To12582912Aligned) {
    for (auto i : {786432, 786433, 786435, 1786432, 999999, 12582911, 12582910, 12582909}) {
        auto data = gen_data(i);
        std::vector<char> data2(i);
        auto mem1 = static_cast<std::byte*>(::operator new (data.size() + 512, std::align_val_t {512}));
        auto mem2 = static_cast<std::byte*>(::operator new (data.size() + 512, std::align_val_t {512}));
        custom_memcpy(mem1, data.data(), data.size());
        custom_memcpy(mem2, mem1, data.size());
        custom_memcpy(data2.data(), mem2, data.size());
        EXPECT_EQ(data, data2) << "size : " << i;
    }
}

TEST(MemCopy, Range12582912To25165824Unaligned) {
    for (auto i : {12582912, 12582919, 25165823, 25165824}) {
        auto data = gen_data(i);
        std::vector<char> data2(i);
        auto mem1 = static_cast<std::byte*>(::operator new (data.size() + 512, std::align_val_t {512}));
        auto mem2 = static_cast<std::byte*>(::operator new (data.size() + 512, std::align_val_t {512}));
        mem1 += 13;
        custom_memcpy(mem1, data.data(), data.size());
        custom_memcpy(mem2, mem1, data.size());
        custom_memcpy(data2.data(), mem2, data.size());
        EXPECT_EQ(data, data2) << "size : " << i;
    }
}

TEST(MemCopy, Range12582912To25165824Aaligned) {
    for (auto i : {12582912, 12582919, 25165823, 25165824}) {
        auto data = gen_data(i);
        std::vector<char> data2(i);
        auto mem1 = static_cast<std::byte*>(::operator new (data.size() + 512, std::align_val_t {512}));
        auto mem2 = static_cast<std::byte*>(::operator new (data.size() + 512, std::align_val_t {512}));
        custom_memcpy(mem1, data.data(), data.size());
        custom_memcpy(mem2, mem1, data.size());
        custom_memcpy(data2.data(), mem2, data.size());
        EXPECT_EQ(data, data2) << "size : " << i;
    }
}