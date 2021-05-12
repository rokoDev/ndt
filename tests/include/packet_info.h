#include "ndt/core.h"

namespace client
{
enum class ePacketType : uint8_t
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
    UserInput() noexcept = default;
    inline explicit UserInput(ndt::BinReader &aReader) noexcept
        : actionKey_(aReader.get<decltype(actionKey_)>())
    {
    }
    inline uint8_t actionKey() const noexcept { return actionKey_; }
    inline void actionKey(const uint8_t aActionKey) noexcept
    {
        actionKey_ = aActionKey;
    }
    constexpr std::size_t minBitSize() const noexcept
    {
        return sizeof(actionKey_) * 8;
    }
    inline void deserialize(ndt::BinReader &aReader) noexcept
    {
        actionKey_ = aReader.get<uint8_t>();
    }
    inline void serialize(ndt::BinWriter &aWriter) const noexcept
    {
        aWriter.add<uint8_t>(actionKey_);
    }

    friend bool operator==(const UserInput &aValue1, const UserInput &aValue2)
    {
        return aValue1.actionKey_ == aValue2.actionKey_;
    }

    friend bool operator!=(const UserInput &aValue1, const UserInput &aValue2)
    {
        return !(aValue1 == aValue2);
    }

   private:
    uint8_t actionKey_ = 0;
};

using JoinContent = ndt::Packet<ndt::PacketId>;
using JoinPacket = ndt::Packet<ndt::TypeId<ePacketType>, JoinContent>;

using LeaveContent = ndt::Packet<ndt::PacketId, ndt::UserId>;
using LeavePacket = ndt::Packet<ndt::TypeId<ePacketType>, LeaveContent>;

using InputContent = ndt::Packet<ndt::PacketId, ndt::UserId, UserInput>;
using InputPacket = ndt::Packet<ndt::TypeId<ePacketType>, InputContent>;

using RequestTimeContent = ndt::Packet<ndt::PacketId, ndt::UserId>;
using RequestTimePacket =
    ndt::Packet<ndt::TypeId<ePacketType>, RequestTimeContent>;

template <ePacketType kPacketT>
class PacketFactory
{
   public:
    static auto create(ndt::BinReader &aReader) noexcept;
};
template <>
class PacketFactory<ePacketType::kJoin>
{
   public:
    static auto create(ndt::BinReader &aReader) noexcept
    {
        return JoinPacket{ndt::TypeId(ePacketType::kJoin),
                          JoinContent(aReader)};
    }
};
template <>
class PacketFactory<ePacketType::kLeave>
{
   public:
    static auto create(ndt::BinReader &aReader) noexcept
    {
        return LeavePacket{ndt::TypeId(ePacketType::kLeave),
                           LeaveContent(aReader)};
    }
};
template <>
class PacketFactory<ePacketType::kInput>
{
   public:
    static auto create(ndt::BinReader &aReader) noexcept
    {
        return InputPacket{ndt::TypeId(ePacketType::kInput),
                           InputContent(aReader)};
    }
};
template <>
class PacketFactory<ePacketType::kRequestTime>
{
   public:
    static auto create(ndt::BinReader &aReader) noexcept
    {
        return RequestTimePacket{ndt::TypeId(ePacketType::kRequestTime),
                                 RequestTimeContent(aReader)};
    }
};
}  // namespace client

namespace server
{
enum class ePacketType : uint8_t
{
    kJoin,
    kGSUpdate,
    kReplyTime,
    Count,
    Error = Count
};

class GameState
{
   public:
    inline uint8_t stateKey() const noexcept { return stateKey_; }
    inline void stateKey(const uint8_t aStateKey) noexcept
    {
        stateKey_ = aStateKey;
    }
    constexpr std::size_t minBitSize() const noexcept
    {
        return sizeof(stateKey_) * 8;
    }
    inline void deserialize(ndt::BinReader &aReader) noexcept
    {
        stateKey_ = aReader.get<uint8_t>();
    }
    inline void serialize(ndt::BinWriter &aWriter) const noexcept
    {
        aWriter.add<uint8_t>(stateKey_);
    }

    friend bool operator==(const GameState &aValue1, const GameState &aValue2)
    {
        return aValue1.stateKey_ == aValue2.stateKey_;
    }

    friend bool operator!=(const GameState &aValue1, const GameState &aValue2)
    {
        return !(aValue1 == aValue2);
    }

   private:
    uint8_t stateKey_ = 0;
};

using JoinPacket =
    ndt::Packet<ndt::TypeId<ePacketType>, ndt::PacketId, ndt::UserId>;
using GSUpdatePacket =
    ndt::Packet<ndt::TypeId<ePacketType>, ndt::PacketId, GameState>;
using ReplyTimePacket =
    ndt::Packet<ndt::TypeId<ePacketType>, ndt::PacketId, ndt::TimeStamp>;
}  // namespace server