#include "gtest/gtest.h"
#include "ndt/buffer.h"

TEST(BufferTests, SaveRead)
{
    char arr[15] = {0};
    ndt::Buffer buf(arr);
    ASSERT_EQ(buf.index(), 0);

    const uint32_t value1 = 123;
    buf.saveInt32(value1);
    uint16_t savedSize = sizeof(value1);
    ASSERT_EQ(buf.index(), savedSize);

    const uint8_t value2 = 100;
    buf.saveInt8(value2);
    savedSize += sizeof(value2);
    ASSERT_EQ(buf.index(), savedSize);

    const uint16_t value3 = 1234;
    buf.saveInt16(value3);
    savedSize += sizeof(value3);
    ASSERT_EQ(buf.index(), savedSize);

    const uint32_t value4 = 123456789;
    buf.saveInt32(value4);
    savedSize += sizeof(value4);
    ASSERT_EQ(buf.index(), savedSize);

    const float value5 = 4321.543f;
    buf.saveFloat(value5);
    savedSize += sizeof(value5);
    ASSERT_EQ(buf.index(), savedSize);

    buf.resetIndex();

    {
        const auto restoredValue = buf.readInt32();
        ASSERT_EQ(restoredValue, value1);
    }

    {
        const auto restoredValue = buf.readInt8();
        ASSERT_EQ(restoredValue, value2);
    }

    {
        const auto restoredValue = buf.readInt16();
        ASSERT_EQ(restoredValue, value3);
    }

    {
        const auto restoredValue = buf.readInt32();
        ASSERT_EQ(restoredValue, value4);
    }

    {
        const auto restoredValue = buf.readFloat();
        ASSERT_EQ(restoredValue, value5);
    }
}