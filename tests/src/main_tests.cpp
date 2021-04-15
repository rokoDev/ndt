#include <fmt/core.h>

#include "gtest/gtest.h"
#include "ndt/utils.h"
#include "ndt/version_info.h"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    const auto testResult = RUN_ALL_TESTS();

    fmt::print("{}", ndt::version_info());
    fmt::print("endian: {}\n", ndt::utils::endianName());

    return testResult;
}