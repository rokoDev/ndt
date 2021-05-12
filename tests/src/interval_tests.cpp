#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ndt/interval.h"
#include "ndt/type_name.h"
#include "ndt/utils.h"

class IntervalTest : public ::testing::Test
{
   protected:
    ndt::Interval<int, ndt::Min<-1>, ndt::Max<2>> interval;
    static constexpr auto inside = ndt::make_array(-1, 0, 1, 2);
    static constexpr auto below = ndt::make_array(-2, -3, -10, -100);
    static constexpr auto above = ndt::make_array(3, 4, 5, 10, 100);
    static constexpr auto outside = ndt::concatenate_arrays(below, above);
};

TEST_F(IntervalTest, MinMax)
{
    ASSERT_EQ(interval.kMin, -1);
    ASSERT_EQ(interval.kMax, 2);
}

TEST_F(IntervalTest, Length) { ASSERT_EQ(interval.length(), inside.size()); }

TEST_F(IntervalTest, Contains)
{
    for (const auto value: outside)
    {
        ASSERT_FALSE(interval.contains(value));
    }

    for (const auto value: inside)
    {
        ASSERT_TRUE(interval.contains(value));
    }
}

TEST_F(IntervalTest, Location)
{
    for (const auto value: below)
    {
        ASSERT_EQ(interval.location(value), ndt::eIntervalLocation::kBelow);
    }

    for (const auto value: above)
    {
        ASSERT_EQ(interval.location(value), ndt::eIntervalLocation::kAbove);
    }

    for (const auto value: inside)
    {
        ASSERT_EQ(interval.location(value), ndt::eIntervalLocation::kInside);
    }
}