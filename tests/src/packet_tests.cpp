#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ndt/packet.h"

template <typename std::size_t BufSize>
class PacketTest : public ::testing::Test
{
   public:
    enum class eClientPacket : uint8_t
    {
        kJoin,
        kLeave,
        kInput,
        kRequestTime,
        Count,
        Error = Count
    };

    class UserInput
    {
       public:
        inline float velocity() const noexcept { return velocity_; }
        inline void velocity(const float aVelocity) noexcept
        {
            velocity_ = aVelocity;
        }

        inline float xDirection() const noexcept { return xDirection_; }
        inline void xDirection(const float aXDirection) noexcept
        {
            xDirection_ = aXDirection;
        }

        inline float yDirection() const noexcept { return yDirection_; }
        inline void yDirection(const float aYDirection) noexcept
        {
            yDirection_ = aYDirection;
        }

        constexpr std::size_t minBitSize() const noexcept
        {
            return (sizeof(velocity_) + sizeof(xDirection_) +
                    sizeof(yDirection_)) *
                   8;
        }

        inline void deserialize(ndt::BinReader &aReader) noexcept
        {
            velocity_ = aReader.get<float>();
            xDirection_ = aReader.get<float>();
            yDirection_ = aReader.get<float>();
        }
        inline void serialize(ndt::BinWriter &aWriter) const noexcept
        {
            aWriter.add<float>(velocity_);
            aWriter.add<float>(xDirection_);
            aWriter.add<float>(yDirection_);
        }

       private:
        float velocity_ = 0.f;
        float xDirection_ = 0.f;
        float yDirection_ = 0.f;
    };

    using JoinPacket =
        ndt::Packet<eClientPacket, eClientPacket::kJoin, ndt::PacketId>;
    using LeavePacket = ndt::Packet<eClientPacket, eClientPacket::kLeave,
                                    ndt::PacketId, ndt::UserId>;
    using InputPacket = ndt::Packet<eClientPacket, eClientPacket::kInput,
                                    ndt::PacketId, ndt::UserId, UserInput>;
    using RequestTimePacket =
        ndt::Packet<eClientPacket, eClientPacket::kRequestTime, ndt::PacketId,
                    ndt::UserId, ndt::TimeStamp>;

    enum class eServerPacket : uint8_t
    {
        kJoin,
        kGameStateUpdate,
        kReplyTime,
        Count,
        Error = Count
    };

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
using PacketTest10 = PacketTest<10>;

TEST_F(PacketTest100, JoinConstructor)
{
    JoinPacket onePacket;

    ASSERT_EQ(onePacket.typeId(), eClientPacket::kJoin);
    ASSERT_EQ(onePacket.packetId(), 0);
}

TEST_F(PacketTest100, JoinSerializeDeserialize)
{
    JoinPacket onePacket;
    onePacket.packetId(11);

    ASSERT_EQ(onePacket.typeId(), eClientPacket::kJoin);
    ASSERT_EQ(onePacket.packetId(), 11);
    ASSERT_EQ(onePacket.minBitSize(), 19);

    ASSERT_EQ(onePacket.serialize(writer), std::error_code());

    const eClientPacket packetType = reader.get<eClientPacket>();
    ASSERT_EQ(packetType, eClientPacket::kJoin);

    JoinPacket restored;
    ASSERT_EQ(restored.deserialize(reader), std::error_code());
    ASSERT_EQ(restored.typeId(), eClientPacket::kJoin);
    ASSERT_EQ(restored.packetId(), 11);
    ASSERT_EQ(restored.minBitSize(), 19);
}

TEST_F(PacketTest100, InputSerializeDeserialize)
{
    InputPacket onePacket;
    onePacket.packetId(11);
    onePacket.velocity(145.43f);
    onePacket.xDirection(0.5f);
    onePacket.yDirection(0.32f);

    ASSERT_EQ(onePacket.serialize(writer), std::error_code());

    const eClientPacket packetType = reader.get<eClientPacket>();
    ASSERT_EQ(packetType, eClientPacket::kInput);

    InputPacket restored;
    ASSERT_EQ(restored.deserialize(reader), std::error_code());
    ASSERT_EQ(restored.typeId(), eClientPacket::kInput);
    ASSERT_EQ(restored.packetId(), 11);
    ASSERT_EQ(restored.userId(), 0);
    ASSERT_FLOAT_EQ(restored.velocity(), 145.43f);
    ASSERT_FLOAT_EQ(restored.xDirection(), 0.5f);
    ASSERT_FLOAT_EQ(restored.yDirection(), 0.32f);
    ASSERT_EQ(restored.minBitSize(), 123);
}

TEST_F(PacketTest10, LackOfSpaceWrite)
{
    InputPacket onePacket;
    ASSERT_EQ(onePacket.serialize(writer),
              std::error_code(ENOMEM, std::system_category()));
}

TEST_F(PacketTest10, LackOfSpaceRead)
{
    InputPacket onePacket;
    ASSERT_EQ(onePacket.deserialize(reader),
              std::error_code(ENOMEM, std::system_category()));
}