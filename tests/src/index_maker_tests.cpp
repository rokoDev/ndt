#include "gtest/gtest.h"
#include "ndt/index_maker.h"

TEST(IndexMakerTest, MaxIndex1)
{
    ndt::IndexMaker<uint8_t, 1> indexMaker;
    for (std::size_t i = 0; i < 100; ++i)
    {
        const auto index = indexMaker.next();
        if (i % (indexMaker.max() + 1) == 0)
        {
            ASSERT_EQ(index, 0);
        }
        else
        {
            ASSERT_EQ(i % (indexMaker.max() + 1), index);
        }
    }
}

TEST(IndexMakerTest, MaxIndex20)
{
    ndt::IndexMaker<uint8_t, 20> indexMaker;
    for (std::size_t i = 0; i < 100; ++i)
    {
        const auto index = indexMaker.next();
        if (i % (indexMaker.max() + 1) == 0)
        {
            ASSERT_EQ(index, 0);
        }
        else
        {
            ASSERT_EQ(i % (indexMaker.max() + 1), index);
        }
    }
}

TEST(IndexMakerTest, Overflow)
{
    ndt::IndexMaker<uint8_t> indexMaker;
    for (std::size_t i = 0; i < 1024; ++i)
    {
        const auto index = indexMaker.next();
        if (i % (indexMaker.max() + 1) == 0)
        {
            ASSERT_EQ(index, 0);
        }
        else
        {
            ASSERT_EQ(i % (indexMaker.max() + 1), index);
        }
    }
}