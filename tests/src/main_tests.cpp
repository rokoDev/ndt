#include <fmt/core.h>
#include <gtest/gtest.h>

#include "ndt/core.h"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    const auto testResult = RUN_ALL_TESTS();

    fmt::print("{}", ndt::version_info());
    fmt::print("actual endian: {}\n", ndt::endianName());

    return testResult;
}