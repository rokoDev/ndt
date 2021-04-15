#include <fmt/core.h>
#include <gmock/gmock.h>

#include <type_traits>

#include "gtest/gtest.h"
#include "ndt/bin_rw.h"

template <typename std::size_t BufSize>
class ByteTest : public ::testing::Test
{
   public:
    enum class eMySuperState
    {
        kState1,
        kState2,
        kState3,
        Count
    };

    ByteTest()
        : rawData_{}
        , writeBuf_(rawData_)
        , writer(writeBuf_)
        , readBuf_(rawData_)
        , reader(readBuf_)
    {
    }

    void printWriterState()
    {
        const std::size_t byteCount =
            writer.byteIndex() + static_cast<bool>(writer.bitIndex());
        for (std::size_t i = 0; i < byteCount; ++i)
        {
            fmt::print("{:08b} ", static_cast<uint8_t>(rawData_[i]));
        }
        fmt::print("\n");
    }

    void printReaderState()
    {
        const std::size_t byteCount =
            reader.byteIndex() + static_cast<bool>(reader.bitIndex());
        for (std::size_t i = 0; i < byteCount; ++i)
        {
            fmt::print("{:08b} ", static_cast<uint8_t>(rawData_[i]));
        }
        fmt::print("\n");
    }

   protected:
    char rawData_[BufSize];
    ndt::Buffer writeBuf_;
    ndt::BinWriter writer;
    ndt::CBuffer readBuf_;
    ndt::BinReader reader;
};

using N1ByteTest = ByteTest<1>;
using N100ByteTest = ByteTest<100>;

TEST_F(N1ByteTest, ReaderConstructor)
{
    ASSERT_EQ(reader.byteIndex(), 0);
    ASSERT_EQ(reader.bitIndex(), 0);
}

TEST_F(N1ByteTest, ReadFirstBit)
{
    rawData_[0] = static_cast<uint8_t>(1 << 7);
    const bool result = reader.get<bool>();
    ASSERT_EQ(result, true);
    ASSERT_EQ(reader.byteIndex(), 0);
    ASSERT_EQ(reader.bitIndex(), 1);
}

TEST_F(N1ByteTest, ReadAllBits)
{
    rawData_[0] = static_cast<uint8_t>(150);
    for (std::size_t i = 0; i < 8; ++i)
    {
        const bool bit = reader.get<bool>();
        const bool validBit =
            static_cast<uint8_t>(rawData_[0] >> (8 - i - 1)) & 1;
        ASSERT_EQ(bit, validBit);
        ASSERT_EQ(reader.byteIndex(), (i + 1) / 8);
        ASSERT_EQ(reader.bitIndex(), (i + 1) % 8);
    }
}

TEST_F(N100ByteTest, WriteReadBoolean)
{
    writer.add(true);
    writer.add(true);
    writer.add(false);
    writer.add(true);

    ASSERT_EQ(writer.byteIndex(), 0);
    ASSERT_EQ(writer.bitIndex(), 4);

    ASSERT_EQ(reader.get<bool>(), true);
    ASSERT_EQ(reader.get<bool>(), true);
    ASSERT_EQ(reader.get<bool>(), false);
    ASSERT_EQ(reader.get<bool>(), true);

    ASSERT_EQ(reader.byteIndex(), 0);
    ASSERT_EQ(reader.bitIndex(), 4);
}

TEST_F(N100ByteTest, WriteReadBoolAndUpTo8Bits)
{
    writer.add(true);
    writer.add(true);
    writer.add(false);
    writer.add(true);
    writer.add<uint8_t>(27, 6);
    writer.add<uint8_t>(13, 4);
    writer.add<uint8_t>(5, 3);

    ASSERT_EQ(writer.byteIndex(), 2);
    ASSERT_EQ(writer.bitIndex(), 1);

    ASSERT_EQ(reader.get<bool>(), true);
    ASSERT_EQ(reader.get<bool>(), true);
    ASSERT_EQ(reader.get<bool>(), false);
    ASSERT_EQ(reader.get<bool>(), true);

    ASSERT_EQ(reader.get<uint8_t>(6), 27);
    ASSERT_EQ(reader.byteIndex(), 1);
    ASSERT_EQ(reader.bitIndex(), 2);

    ASSERT_EQ(reader.get<uint8_t>(4), 13);
    ASSERT_EQ(reader.byteIndex(), 1);
    ASSERT_EQ(reader.bitIndex(), 6);

    ASSERT_EQ(reader.get<uint8_t>(3), 5);
    ASSERT_EQ(reader.byteIndex(), 2);
    ASSERT_EQ(reader.bitIndex(), 1);
}

TEST_F(N100ByteTest, WriteReadUInt8)
{
    writer.add<uint8_t>(120);
    writer.add<uint8_t>(255);
    writer.add<uint8_t>(0);
    writer.add<uint8_t>(1);
    writer.add<uint8_t>(100);

    ASSERT_EQ(writer.byteIndex(), 5);
    ASSERT_EQ(writer.bitIndex(), 0);

    ASSERT_EQ(reader.get<uint8_t>(), 120);
    ASSERT_EQ(reader.get<uint8_t>(), 255);
    ASSERT_EQ(reader.get<uint8_t>(), 0);
    ASSERT_EQ(reader.get<uint8_t>(), 1);
    ASSERT_EQ(reader.get<uint8_t>(), 100);

    ASSERT_EQ(reader.byteIndex(), 5);
    ASSERT_EQ(reader.bitIndex(), 0);
}

TEST_F(N100ByteTest, WriteReadUInt16)
{
    writer.add<uint16_t>(1200);
    writer.add<uint16_t>(2055);
    writer.add<uint16_t>(0);
    writer.add<uint16_t>(10);
    writer.add<uint16_t>(100);
    writer.add<uint16_t>(54300);

    ASSERT_EQ(writer.byteIndex(), 12);
    ASSERT_EQ(writer.bitIndex(), 0);

    ASSERT_EQ(reader.get<uint16_t>(), 1200);
    ASSERT_EQ(reader.get<uint16_t>(), 2055);
    ASSERT_EQ(reader.get<uint16_t>(), 0);
    ASSERT_EQ(reader.get<uint16_t>(), 10);
    ASSERT_EQ(reader.get<uint16_t>(), 100);
    ASSERT_EQ(reader.get<uint16_t>(), 54300);

    ASSERT_EQ(reader.byteIndex(), 12);
    ASSERT_EQ(reader.bitIndex(), 0);
}

TEST_F(N100ByteTest, WriteReadUInt16AndUpTo8Bits)
{
    writer.add<uint16_t>(1200);
    writer.add<uint8_t>(27, 6);
    writer.add<uint16_t>(2055);
    writer.add<uint16_t>(0);
    writer.add<uint8_t>(9, 5);
    writer.add<uint16_t>(10);
    writer.add<uint16_t>(100);
    writer.add<uint8_t>(41, 6);
    writer.add<uint16_t>(54300);
    writer.add<uint8_t>(73, 7);

    ASSERT_EQ(reader.get<uint16_t>(), 1200);
    ASSERT_EQ(reader.get<uint8_t>(6), 27);
    ASSERT_EQ(reader.get<uint16_t>(), 2055);
    ASSERT_EQ(reader.get<uint16_t>(), 0);
    ASSERT_EQ(reader.get<uint8_t>(5), 9);
    ASSERT_EQ(reader.get<uint16_t>(), 10);
    ASSERT_EQ(reader.get<uint16_t>(), 100);
    ASSERT_EQ(reader.get<uint8_t>(6), 41);
    ASSERT_EQ(reader.get<uint16_t>(), 54300);
    ASSERT_EQ(reader.get<uint8_t>(7), 73);
}

TEST_F(N100ByteTest, WriteReadUInt16AndUpTo16Bits)
{
    writer.add<uint16_t>(1200);
    writer.add<uint8_t>(40, 6);
    writer.add<uint32_t>(185691945, 28);
    writer.add<uint16_t>(2055);
    writer.add<uint32_t>(460011, 19);
    writer.add<uint16_t>(0);
    writer.add<uint8_t>(9, 5);
    writer.add<uint16_t>(10);
    writer.add<uint16_t>(100);
    writer.add<int8_t>(-102);
    writer.add<uint8_t>(41, 6);
    writer.add<uint8_t>(22, 5);
    writer.add<uint32_t>(-47463890);
    writer.add<uint16_t>(54300);
    writer.add<uint8_t>(73, 7);
    writer.add<float>(-7433.459f);
    writer.add<uint64_t>(UINT64_C(10524353777411039616));
    writer.add<bool>(true);
    writer.add<double>(74787833.459);
    writer.add(eMySuperState::kState2);
    writer.add<bool>(false);
    writer.add<uint8_t>(22, 5);
    writer.add<uint16_t>(54300);
    writer.add(eMySuperState::kState3);
    writer.add<uint8_t>(73, 7);

    ASSERT_EQ(reader.get<uint16_t>(), 1200);
    ASSERT_EQ(reader.get<uint8_t>(6), 40);
    ASSERT_EQ(reader.get<uint32_t>(28), 185691945);
    ASSERT_EQ(reader.get<uint16_t>(), 2055);
    ASSERT_EQ(reader.get<uint32_t>(19), 460011);
    ASSERT_EQ(reader.get<uint16_t>(), 0);
    ASSERT_EQ(reader.get<uint8_t>(5), 9);
    ASSERT_EQ(reader.get<uint16_t>(), 10);
    ASSERT_EQ(reader.get<uint16_t>(), 100);
    ASSERT_EQ(reader.get<int8_t>(), -102);
    ASSERT_EQ(reader.get<uint8_t>(6), 41);
    ASSERT_EQ(reader.get<uint8_t>(5), 22);
    ASSERT_EQ(reader.get<int32_t>(), -47463890);
    ASSERT_EQ(reader.get<uint16_t>(), 54300);
    ASSERT_EQ(reader.get<uint8_t>(7), 73);
    ASSERT_FLOAT_EQ(reader.get<float>(), -7433.459f);
    ASSERT_EQ(reader.get<uint64_t>(), UINT64_C(10524353777411039616));
    ASSERT_EQ(reader.get<bool>(), true);
    ASSERT_DOUBLE_EQ(reader.get<double>(), 74787833.459);
    const auto bitsIndex = reader.bitIndex();
    const auto byteIndex = reader.byteIndex();
    ASSERT_EQ(reader.get<eMySuperState>(), eMySuperState::kState2);
    ASSERT_EQ(reader.bitIndex(), (bitsIndex + 2) % 8);
    ASSERT_EQ(reader.byteIndex(), byteIndex + (bitsIndex + 2) / 8);
    ASSERT_EQ(reader.get<bool>(), false);
    ASSERT_EQ(reader.get<uint8_t>(5), 22);
    ASSERT_EQ(reader.get<uint16_t>(), 54300);
    ASSERT_EQ(reader.get<eMySuperState>(), eMySuperState::kState3);
    ASSERT_EQ(reader.get<uint8_t>(7), 73);
}

TEST_F(N100ByteTest, WriteReadRefsAndConstRefs)
{
    writer.add(true);
    const int16_t& val1 = -1903;
    writer.add(val1);
    ASSERT_EQ(writer.byteIndex(), 2);
    ASSERT_EQ(writer.bitIndex(), 1);
    writer.add<float>(439398.34f);
    ASSERT_EQ(writer.byteIndex(), 6);
    ASSERT_EQ(writer.bitIndex(), 1);
    double testDouble = -9384898.2332;
    writer.add(std::move(testDouble));
    double& doubleRef = testDouble;
    writer.add(doubleRef);
    writer.add<char>('k');
    writer.add<wchar_t>(L'A');  // depending on compiler can take 2 or 4 bytes

    ASSERT_EQ(reader.get<bool>(), true);
    ASSERT_EQ(reader.get<int16_t>(), -1903);
    ASSERT_EQ(reader.byteIndex(), 2);
    ASSERT_EQ(reader.bitIndex(), 1);
    ASSERT_FLOAT_EQ(reader.get<float>(), 439398.34f);
    ASSERT_EQ(reader.byteIndex(), 6);
    ASSERT_EQ(reader.bitIndex(), 1);
    ASSERT_DOUBLE_EQ(reader.get<double>(), testDouble);
    ASSERT_DOUBLE_EQ(reader.get<double>(), doubleRef);
    ASSERT_EQ(reader.get<char>(), 'k');
    ASSERT_EQ(reader.get<wchar_t>(), L'A');
}