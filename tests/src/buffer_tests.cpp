#include "gtest/gtest.h"
#include "ndt/buffer.h"

TEST(BufferTests, SaveRead)
{
    char arr[100] = {0};
    ndt::Buffer buf(arr);

    const uint32_t value1 = 123;
    buf.saveInt32(value1);

    const uint8_t value2 = 100;
    buf.saveInt8(value2);

    const uint16_t value3 = 1234;
    buf.saveInt16(value3);

    const uint32_t value4 = 123456789;
    buf.saveInt32(value4);

    const float value5 = 4321.543f;
    buf.saveFloat(value5);

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