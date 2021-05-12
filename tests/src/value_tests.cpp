#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ndt/utils.h"
#include "ndt/value.h"

TEST(ValueTest, Create)
{
    using ValueT = ndt::Value<int8_t, ndt::Min<static_cast<int8_t>(-10)>,
                              ndt::Max<static_cast<int8_t>(20)>>;
    ValueT val;
    ASSERT_EQ(val.get(), -10);
}