#include <fmt/core.h>

#include "ndt/GitInfo.h"
#include "ndt/utils.h"
#include "ndt/Version.h"
#include "gtest/gtest.h"

TEST(TestNDTVersion, GitInfoTest)
{
    fmt::print("[          ] build type: {}\n", ndt::kBuildTypeCStr);
    fmt::print("[          ] branch: {}\n", ndt::GitInfo::branch());
    fmt::print("[          ] SHA1: {}\n", ndt::GitInfo::SHA1());
    const std::string isDirty =
        ndt::GitInfo::isDirty() ? std::string("true") : std::string("false");
    fmt::print("[          ] IS_DIRTY: {}\n", isDirty);
    ASSERT_EQ(0, 0);
}

TEST(TestNDTVersion, VersionTest)
{
    fmt::print("[          ] ndt Version: {}\n", ndt::Version::str());
    ASSERT_EQ(0, 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
