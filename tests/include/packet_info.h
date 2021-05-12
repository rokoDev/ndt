#ifndef ndt_packet_info_h
#define ndt_packet_info_h

#include <chrono>

#include "ndt/bin_rw.h"
#include "ndt/value.h"

namespace packet_info
{
template <auto>
struct Packet;

enum class eClient : uint8_t
{
    kJoin,
    kLeave,
    kInput,
    kRequestTime,
    Count,
    Error = Count
};

template <typename IndexT, IndexT kMaxSize, typename DataT = unsigned char,
          typename = ndt::UInt<IndexT>>
struct DataArray
{
    static constexpr IndexT kMaxLen = kMaxSize;

   private:
    IndexT size;

   public:
    // constexpr DataArray() = default;
    DataT data[kMaxSize];

    void setSize(IndexT aSize) noexcept
    {
        assert(validateSize(aSize) && "error: invalid size");
        size = aSize;
    }

    DataT& operator[](IndexT aIdx) noexcept
    {
        assert(validateIndex(aIdx) && "error: invalid index");
        return data[aIdx];
    }
    const DataT& operator[](IndexT aIdx) const noexcept
    {
        assert(validateIndex(aIdx) && "error: invalid index");
        return data[aIdx];
    }

    inline constexpr bool validateSize(IndexT aSize) const noexcept
    {
        return aSize <= kMaxLen;
    }

    inline bool validateIndex(IndexT aIdx) const noexcept
    {
        return (aIdx < size) && validateSize(size);
    }
};

template <>
struct Packet<eClient::kJoin>
{
    uint16_t packetId;
};

template <>
struct Packet<eClient::kLeave>
{
    uint16_t packetId;
    uint8_t userId;
    ndt::Value<int, ndt::Min<-1100>, ndt::Max<-1000>> level;
    std::chrono::microseconds mcDur{89123};

    friend bool operator==(const Packet<eClient::kLeave>& aVal1,
                           const Packet<eClient::kLeave>& aVal2)
    {
        return (aVal1.packetId == aVal2.packetId) &&
               (aVal1.userId == aVal2.userId) && (aVal1.level == aVal2.level);
    }
};

template <>
struct Packet<eClient::kInput>
{
    uint16_t packetId;
    uint8_t userId;
    uint8_t actionKey;
};

template <>
struct Packet<eClient::kRequestTime>
{
    uint16_t packetId;
    uint8_t userId;
};

template <>
struct Packet<eClient::Error>
{
};

struct MyTest
{
    uint8_t factor;
    Packet<eClient::kInput> userInput;
    DataArray<uint16_t, 100> blob;
};

enum class eServer : uint8_t
{
    kJoin,
    kGameStateDiff,
    kReplyTime,
    Count,
    Error = Count
};

template <>
struct Packet<eServer::kJoin>
{
    uint16_t packetId;
    uint8_t userId;
};

template <>
struct Packet<eServer::kGameStateDiff>
{
    uint16_t packetId;
    uint8_t gameState;
};

template <>
struct Packet<eServer::kReplyTime>
{
    uint16_t packetId;
    std::chrono::microseconds dtFromStart;
};

template <>
struct Packet<eServer::Error>
{
};

}  // namespace packet_info

#endif /* ndt_packet_info_h */