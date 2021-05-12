#include <fmt/core.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/pfr.hpp>

#include "ndt/bin_rw.h"
#include "ndt/serialize.h"
#include "ndt/type_name.h"
#include "ndt/utils.h"
#include "packet_info.h"

namespace pfr = boost::pfr;

namespace ndt
{
template <typename EnumT, EnumT EnumV>
struct PacketType;

template <auto EnumV>
struct PacketType<packet_info::eClient, EnumV>
{
    using type = packet_info::Packet<EnumV>;
};

template <auto EnumV>
using PacketT = typename PacketType<decltype(EnumV), EnumV>::type;

template <typename EnumT, std::size_t EnumVAsIndex>
decltype(auto) deserializePacket(BinReader &aReader,
                                 std::error_code &aEc) noexcept
{
    using namespace packet_info;
    constexpr EnumT kEnumV = static_cast<EnumT>(EnumVAsIndex);
    return deserialize<PacketT<kEnumV>>(aReader, aEc);
}

template <typename EnumT, std::size_t... i>
constexpr auto make_handlers_tuple(std::index_sequence<i...>) noexcept
    -> std::tuple<decltype(&deserializePacket<EnumT, i>)...>
{
    return {&deserializePacket<EnumT, i>...};
}

using PacketReaderPtr = void (*)(BinReader &aReader,
                                 std::error_code &aEc) noexcept;

template <typename EnumT, std::size_t EnumVAsIndex>
void deserializePacket2(BinReader &aReader, std::error_code &aEc) noexcept
{
    constexpr EnumT kEnumV = static_cast<EnumT>(EnumVAsIndex);
    PacketT<kEnumV> packetRef = deserialize<PacketT<kEnumV>>(aReader, aEc);
}

template <typename EnumT, std::size_t... i>
constexpr auto make_handlers_array(std::index_sequence<i...>) noexcept
    -> std::array<PacketReaderPtr, sizeof...(i)>
{
    return {&deserializePacket2<EnumT, i>...};
}

template <typename EnumT,
          typename = std::enable_if_t<std::is_enum_v<std::decay_t<EnumT>>>>
decltype(auto) readPacket(BinReader &aReader, std::error_code &aEc) noexcept
{
    static_assert(utils::to_underlying(EnumT::Error) ==
                      utils::to_underlying(EnumT::Count),
                  "Enum must have values E::Count and E::Error which must be "
                  "equal after converting to underlying type.");
    const auto kPacketType = aReader.get<EnumT>(aEc);
    using Indices =
        std::make_index_sequence<static_cast<std::size_t>(EnumT::Count)>;
    constexpr auto handlersArray = make_handlers_array<EnumT>(Indices{});
    return handlersArray[static_cast<std::size_t>(kPacketType)](aReader, aEc);
}
}  // namespace ndt

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
    char rawData_[BufSize] = {0};
    ndt::Buffer writeBuf_;
    ndt::BinWriter writer;
    ndt::CBuffer readBuf_;
    ndt::BinReader reader;
};

using PacketTest100 = PacketTest<100>;
using PacketTest3 = PacketTest<3>;
using PacketTest1 = PacketTest<1>;

TEST_F(PacketTest1, InputSerializeLowMemory)
{
    using namespace packet_info;
    constexpr Packet<eClient::kInput> kSerializedPacket{11, 5, 222};

    auto ec = ndt::serialize(writer, kSerializedPacket);
    ASSERT_NE(ec.value(), 0);
}

TEST_F(PacketTest100, JoinSerializeDeserialize)
{
    using namespace packet_info;
    constexpr uint16_t kPacketId = 11;
    constexpr Packet<eClient::kJoin> onePacket{kPacketId};

    constexpr auto kNumBitsForPacketType =
        ndt::utils::enum_properties<eClient>::numBits;
    constexpr auto kNumBitsJoin =
        kNumBitsForPacketType + ndt::utils::num_bits<decltype(kPacketId)>();

    auto ec = ndt::serialize(writer, onePacket);
    ASSERT_EQ(ec.value(), 0);
    ASSERT_EQ(writer.bitSize(), kNumBitsJoin);

    const auto kPacketType = reader.get<eClient>(ec);
    ASSERT_EQ(ec.value(), 0);
    ASSERT_EQ(kPacketType, eClient::kJoin);

    const auto restored = ndt::deserialize<Packet<eClient::kJoin>>(reader, ec);
    ASSERT_EQ(ec.value(), 0);
    ASSERT_TRUE(pfr::eq(onePacket, restored));
    ASSERT_EQ(reader.bitSize(), kNumBitsJoin);
}

TEST_F(PacketTest100, InputSerializeDeserialize)
{
    using namespace packet_info;
    constexpr Packet<eClient::kInput> kSerializedPacket{11, 5, 222};

    auto ec = ndt::serialize(writer, kSerializedPacket);
    ASSERT_EQ(ec.value(), 0);

    const auto kPacketType = reader.get<eClient>(ec);
    ASSERT_EQ(ec.value(), 0);
    ASSERT_EQ(kPacketType, eClient::kInput);

    const auto kDeserializedPacket =
        ndt::deserialize<Packet<eClient::kInput>>(reader, ec);
    ASSERT_EQ(ec.value(), 0);
    ASSERT_TRUE(pfr::eq(kSerializedPacket, kDeserializedPacket));
}

TEST_F(PacketTest100, SerializationWithValue)
{
    using namespace packet_info;
    constexpr auto kNumBitsForPacketType =
        ndt::utils::enum_properties<eClient>::numBits;
    constexpr auto kBitsInPacket = 3 * 8 + 7 + kNumBitsForPacketType + 64;
    Packet<eClient::kLeave> leavePacket{2222, 111};
    ASSERT_EQ(leavePacket.level.get(), -1100);
    leavePacket.level.set(-1099);
    std::error_code ec = ndt::serialize(writer, leavePacket);
    ASSERT_EQ(ec.value(), 0);
    ASSERT_EQ(writer.bitSize(), kBitsInPacket);

    const auto kPacketType = reader.get<eClient>(ec);
    ASSERT_EQ(ec.value(), 0);
    ASSERT_EQ(kPacketType, eClient::kLeave);
    const auto restoredLeavePacket =
        ndt::deserialize<Packet<eClient::kLeave>>(reader, ec);
    ASSERT_EQ(ec.value(), 0);
    ASSERT_EQ(leavePacket, restoredLeavePacket);
    ASSERT_EQ(reader.bitSize(), kBitsInPacket);
}

TEST_F(PacketTest3, LackOfSpaceWrite)
{
    using namespace packet_info;
    constexpr Packet<eClient::kInput> onePacket{};
    const auto ec = ndt::serialize(writer, onePacket);
    ASSERT_NE(ec.value(), 0);
}

TEST_F(PacketTest3, LackOfSpaceRead)
{
    using namespace packet_info;
    std::error_code ec;
    ndt::deserialize<Packet<eClient::kInput>>(reader, ec);
    ASSERT_NE(ec.value(), 0);
}