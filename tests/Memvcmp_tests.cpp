#include "NetUtils.h"
#include "gtest/gtest.h"

TEST(Memvcmp_TESTS, NonEmptyMemTest)
{
    const char arr[10] = {0};
    const auto result =
        net::utils::memvcmp(static_cast<const void *>(arr), 0, sizeof(arr));
    ASSERT_EQ(result, true);
}

TEST(Memvcmp_TESTS, NullptrMemTest_ZeroSize)
{
    const char *arr = nullptr;
    const auto result =
        net::utils::memvcmp(static_cast<const void *>(arr), 0, 0);
    ASSERT_EQ(result, false);
}

TEST(Memvcmp_TESTS, NullptrMemTest_NonZeroSize)
{
    const char *arr = nullptr;
    const auto result =
        net::utils::memvcmp(static_cast<const void *>(arr), 0, 10);
    ASSERT_EQ(result, false);
}

TEST(Memvcmp_TESTS, NonNullptrMemTest_OneSize)
{
    const char arr[1] = {0};
    const auto result =
        net::utils::memvcmp(static_cast<const void *>(arr), 0, sizeof(arr));
    ASSERT_EQ(result, true);
}