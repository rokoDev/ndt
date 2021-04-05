#include <fmt/core.h>

#include "gtest/gtest.h"
#include "ndt/endian.h"

TEST(EndianTest, AssumedEndianEqualActual)
{
    ASSERT_EQ(ndt::kEndian /*assumed endian*/, ndt::endian() /*actual endian*/);
}