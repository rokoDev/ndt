#include <fmt/core.h>

#include <initializer_list>
#include <limits>
#include <tuple>
#include <utility>

#include "gtest/gtest.h"
#include "ndt/buffer.h"

template <typename F, typename... Ts>
void for_each_arg(F&& f, Ts&&... ts)
{
    using I = std::initializer_list<int>;
    (void)I{(std::forward<F>(f)(std::forward<Ts>(ts)), 0)...};
}

template <typename F, typename T>
void for_each_in_tuple(F&& f, T&& t)
{
    std::apply(
        [&f](auto&&... xs) {
            for_each_arg(f, std::forward<decltype(xs)>(xs)...);
        },
        std::forward<T>(t));
}

template <typename T>
void assert_eq(const T arg1, const T arg2)
{
    ASSERT_EQ(arg1, arg2);
}

template <>
void assert_eq<float>(const float arg1, const float arg2)
{
    ASSERT_FLOAT_EQ(arg1, arg2);
}

template <typename T>
void testDataInTuple(T&& testData)
{
    char arr[sizeof testData] = {0};
    ndt::Buffer buf(arr);
    ndt::BufferWriter writer(buf);

    uint16_t savedSize = 0;
    uint8_t bits = 0;
    for_each_in_tuple(
        [&writer, &savedSize, &bits](const auto value) {
            writer.add(value);
            if constexpr (std::is_same_v<
                              bool, typename std::decay_t<decltype(value)>>)
            {
                savedSize += (bits + 1) / 8;
                bits = (bits + 1) % 8;
                assert_eq(writer.bitIndex(), bits);
            }
            else
            {
                savedSize += sizeof(value);
            }
            assert_eq(writer.byteIndex(), savedSize);
        },
        testData);

    ndt::BufferReader reader((ndt::CBuffer(buf)));

    for_each_in_tuple(
        [&reader](const auto value) {
            const auto restoredValue = reader.get<decltype(value)>();
            assert_eq(restoredValue, value);
        },
        testData);
}

TEST(BufferTests, SaveRead)
{
    using TestDataT = std::tuple<uint32_t, uint8_t, uint16_t, uint32_t, float>;
    TestDataT testData{123, 100, 1234, 123456789, 4321.543f};
    testDataInTuple(testData);
}

TEST(BufferTests, SaveReadNegative)
{
    using TestDataT = std::tuple<uint32_t, int8_t, int16_t, int32_t, float,
                                 char, int, int, uint8_t, float, float>;
    TestDataT testData{123,
                       -100,
                       -1234,
                       -123456789,
                       -4321.543f,
                       -123,
                       std::numeric_limits<int>::min(),
                       std::numeric_limits<int>::max(),
                       0,
                       std::numeric_limits<float>::min(),
                       std::numeric_limits<float>::max()};
    testDataInTuple(testData);
}

TEST(BufferTests, SaveReadBoolOnly)
{
    using TestDataT = std::tuple<bool, bool, bool, bool, bool, bool, bool, bool,
                                 bool, bool, bool>;
    TestDataT testData{true,  false, true, true,  false, false,
                       false, true,  true, false, true};
    testDataInTuple(testData);
}