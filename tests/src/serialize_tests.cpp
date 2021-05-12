#include <fmt/core.h>
#include <gtest/gtest.h>

#include "ndt/serialize.h"

template <typename std::size_t BufSize>
class SerializeTest : public ::testing::Test
{
   public:
    enum class eMySuperState : uint32_t
    {
        kState1,
        kState2,
        kState3,
        kState4,
        Count,
        Error = Count
    };

    SerializeTest()
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

using N100ByteSerializeTest = SerializeTest<100>;

TEST_F(N100ByteSerializeTest, BoolValue)
{
    const auto ec = ndt::serialize(writer, true);
    ASSERT_EQ(ec.value(), 0);
}