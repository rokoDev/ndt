#include <fmt/core.h>

#include "gtest/gtest.h"
#include "ndt/version_info.h"

TEST(TestNDTVersion, VersionTest)
{
    fmt::print("{}", ndt::version_info());
    ASSERT_EQ(0, 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}