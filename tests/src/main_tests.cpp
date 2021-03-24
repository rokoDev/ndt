#include <fmt/core.h>

#include "gtest/gtest.h"
#include "ndt/version_info.h"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    fmt::print("{}", ndt::version_info());

    return RUN_ALL_TESTS();
}