#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "packet_info.h"

template <typename std::size_t BufSize>
class PacketTest : public ::testing::Test
{
   public:

    PacketTest()
        : rawData_{}
        , writeBuf_(rawData_)
        , writer(writeBuf_)
        , readBuf_(rawData_)
        , reader(readBuf_)
    {
    }

   protected:
    char rawData_[BufSize];
    ndt::Buffer writeBuf_;
    ndt::BinWriter writer;
    ndt::CBuffer readBuf_;
    ndt::BinReader reader;
};

using PacketTest100 = PacketTest<100>;
using PacketTest3 = PacketTest<3>;

TEST_F(PacketTest100, JoinConstructor)
{
    client::JoinPacket onePacket{ndt::TypeId(client::ePacketType::kJoin)};

    ASSERT_EQ(onePacket.typeId(), client::ePacketType::kJoin);
    ASSERT_EQ(onePacket.packetId(), 0);
}

TEST_F(PacketTest100, JoinSerializeDeserialize)
{
    client::JoinPacket onePacket{ndt::TypeId(client::ePacketType::kJoin)};
    onePacket.packetId(11);

    ASSERT_EQ(onePacket.typeId(), client::ePacketType::kJoin);
    ASSERT_EQ(onePacket.packetId(), 11);
    ASSERT_EQ(onePacket.minBitSize(), 19);
    ASSERT_EQ(onePacket.serialize(writer), std::error_code());

    client::JoinPacket restored;
    ASSERT_EQ(restored.deserialize(reader), std::error_code());
    ASSERT_EQ(onePacket, restored);
    ASSERT_EQ(restored.minBitSize(), 19);
}

TEST_F(PacketTest100, InputSerializeDeserialize)
{
    client::InputPacket onePacket{ndt::TypeId(client::ePacketType::kInput)};
    onePacket.packetId(11);
    onePacket.actionKey(222);
    ASSERT_EQ(onePacket.serialize(writer), std::error_code());

    client::InputPacket restored;
    ASSERT_EQ(restored.deserialize(reader), std::error_code());
    ASSERT_EQ(onePacket, restored);
    ASSERT_EQ(restored.minBitSize(), 35);
}

TEST_F(PacketTest3, LackOfSpaceWrite)
{
    client::InputPacket onePacket;
    ASSERT_EQ(onePacket.serialize(writer),
              std::error_code(ENOMEM, std::system_category()));
}

TEST_F(PacketTest3, LackOfSpaceRead)
{
    client::InputPacket onePacket;
    ASSERT_EQ(onePacket.deserialize(reader),
              std::error_code(ENOMEM, std::system_category()));
}