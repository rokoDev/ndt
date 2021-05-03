#ifndef ndt_packet_h
#define ndt_packet_h

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <system_error>
#include <type_traits>

#include "bin_rw.h"

namespace ndt
{
template <typename EnumT>
class TypeId
{
   public:
    inline explicit TypeId(const EnumT aTypeId) : typeId_(aTypeId)
    {
        static_assert(std::is_enum_v<EnumT>, "EnumT must be enum");
    }

    inline explicit TypeId() : TypeId(EnumT::Error) {}

    EnumT typeId() const noexcept { return typeId_; }
    void typeId(const EnumT aTypeId) noexcept { typeId_ = aTypeId; }

    constexpr std::size_t minBitSize() const noexcept
    {
        return utils::enum_properties<EnumT>::numBits;
    }

    void deserialize(BinReader &aReader) noexcept
    {
        typeId_ = aReader.get<decltype(typeId_)>();
    }
    void serialize(BinWriter &aWriter) const noexcept
    {
        aWriter.add<decltype(typeId_)>(typeId_);
    }

   private:
    EnumT typeId_ = EnumT::Error;
};

class PacketId
{
   public:
    inline explicit PacketId(const uint16_t aPacketId) : packetId_(aPacketId) {}
    PacketId();

    inline uint16_t packetId() const noexcept { return packetId_; }
    inline void packetId(uint16_t aPacketId) noexcept { packetId_ = aPacketId; }
    constexpr std::size_t minBitSize() const noexcept
    {
        return sizeof(packetId_) * 8;
    }

    inline void deserialize(BinReader &aReader) noexcept
    {
        packetId_ = aReader.get<decltype(packetId_)>();
    }
    inline void serialize(BinWriter &aWriter) const noexcept
    {
        aWriter.add<decltype(packetId_)>(packetId_);
    }

   private:
    uint16_t packetId_ = 0;
};

class UserId
{
   public:
    inline explicit UserId(const uint8_t aUserId) : userId_(aUserId) {}
    UserId();

    inline uint8_t userId() const noexcept { return userId_; }
    inline void userId(uint8_t aUserId) noexcept { userId_ = aUserId; }
    constexpr std::size_t minBitSize() const noexcept
    {
        return sizeof(userId_) * 8;
    }

    inline void deserialize(BinReader &aReader) noexcept
    {
        userId_ = aReader.get<decltype(userId_)>();
    }
    inline void serialize(BinWriter &aWriter) const noexcept
    {
        aWriter.add<decltype(userId_)>(userId_);
    }

   private:
    uint8_t userId_ = 0;
};

class TimeStamp
{
   public:
    inline explicit TimeStamp(const std::chrono::microseconds aMc) : mc_(aMc) {}
    TimeStamp();

    inline std::chrono::microseconds timeStamp() const noexcept { return mc_; }
    inline void timeStamp(std::chrono::microseconds aMc) noexcept { mc_ = aMc; }
    constexpr std::size_t minBitSize() const noexcept
    {
        return sizeof(int64_t) * 8;
    }

    inline void deserialize(BinReader &aReader) noexcept
    {
        mc_ = std::chrono::microseconds(aReader.get<int64_t>());
    }
    inline void serialize(BinWriter &aWriter) const noexcept
    {
        aWriter.add<int64_t>(mc_.count());
    }

   private:
    std::chrono::microseconds mc_ = std::chrono::microseconds(0);
};

template <typename EnumT, EnumT packetType, typename... Mixins>
class Packet
    : public TypeId<EnumT>
    , public Mixins...
{
   public:
    Packet() : TypeId<EnumT>(packetType), Mixins()... {}

    [[nodiscard]] constexpr std::size_t minBitSize() const noexcept
    {
        return static_cast<TypeId<EnumT> const *>(this)->minBitSize() +
               (... + static_cast<Mixins const *>(this)->minBitSize());
    }

    [[nodiscard]] std::error_code deserialize(BinReader &aReader) noexcept
    {
        if (minBitSize() <= aReader.bitCapacity())
        {
            return deserializeImpl<Mixins...>(aReader);
        }
        else
        {
            // aReader(received data) is too small to represent this packet
            return std::error_code(ENOMEM, std::system_category());
        }
    }

    [[nodiscard]] std::error_code serialize(BinWriter &aWriter) noexcept
    {
        if (minBitSize() <= aWriter.bitCapacity())
        {
            return serializeImpl<TypeId<EnumT>, Mixins...>(aWriter);
        }
        else
        {
            // aWriter is too small to represent this packet
            return std::error_code(ENOMEM, std::system_category());
        }
    }

   private:
    template <typename Arg, typename... Args>
    std::error_code deserializeImpl(BinReader &aReader)
    {
        using ResultT =
            decltype((static_cast<Arg *>(this))->deserialize(aReader));
        static_assert(std::is_same_v<ResultT, void> ||
                          std::is_same_v<ResultT, std::error_code>,
                      "deserialize must return void or std::error_code");
        if constexpr (std::is_same_v<ResultT, void>)
        {
            (static_cast<Arg *>(this))->deserialize(aReader);
            return deserializeRestImpl<sizeof...(Args), Args...>(aReader);
        }
        else
        {
            const auto result =
                (static_cast<Arg *>(this))->deserialize(aReader);
            if (!result)
            {
                return deserializeRestImpl<sizeof...(Args), Args...>(aReader);
            }
            else
            {
                return result;
            }
        }
    }

    template <size_t numArgs, typename... Args>
    std::error_code deserializeRestImpl(BinReader &aReader)
    {
        if constexpr (numArgs > 0)
        {
            return deserializeImpl<Args...>(aReader);
        }
        else
        {
            return std::error_code();
        }
    }

    template <typename Arg, typename... Args>
    std::error_code serializeImpl(BinWriter &aWriter) const
    {
        using ResultT =
            decltype((static_cast<Arg const *>(this))->serialize(aWriter));
        static_assert(std::is_same_v<ResultT, void> ||
                          std::is_same_v<ResultT, std::error_code>,
                      "deserialize must return void or std::error_code");
        if constexpr (std::is_same_v<ResultT, void>)
        {
            (static_cast<Arg const *>(this))->serialize(aWriter);
            return serializeRestImpl<sizeof...(Args), Args...>(aWriter);
        }
        else
        {
            const auto result =
                (static_cast<Arg const *>(this))->serialize(aWriter);
            if (!result)
            {
                return serializeRestImpl<sizeof...(Args), Args...>(aWriter);
            }
            else
            {
                return result;
            }
        }
    }

    template <size_t numArgs, typename... Args>
    std::error_code serializeRestImpl(BinWriter &aWriter) const
    {
        if constexpr (numArgs > 0)
        {
            return serializeImpl<Args...>(aWriter);
        }
        else
        {
            return std::error_code();
        }
    }
};
}  // namespace ndt

#endif /* ndt_packet_h */