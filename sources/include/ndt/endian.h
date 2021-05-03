#ifndef ndt_endian_h
#define ndt_endian_h

#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>

#include "useful_base_types.h"

#if defined(__linux__)

#include <byteswap.h>
#define swap16(arg) __bswap_16(arg)
#define swap32(arg) __bswap_32(arg)
#define swap64(arg) __bswap_64(arg)

#elif defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#define swap16(arg) OSSwapInt16(arg)
#define swap32(arg) OSSwapInt32(arg)
#define swap64(arg) OSSwapInt64(arg)

#elif defined(_MSC_VER)

#include <stdlib.h>
#define swap16(arg) _byteswap_ushort(arg)
#define swap32(arg) _byteswap_ulong(arg)
#define swap64(arg) _byteswap_uint64(arg)

#else

#define swap16(arg) \
    static_cast<uint16_t>((((arg)&0xff00) >> 8) | (((arg)&0x00ff) << 8))

#define swap32(arg)                                              \
    static_cast<uint32_t>(                                       \
        (((arg)&0xff000000) >> 24) | (((arg)&0x00ff0000) >> 8) | \
        (((arg)&0x0000ff00) << 8) | (((arg)&0x000000ff) << 24))

#define swap64(arg)                                               \
    static_cast<uint64_t>((((arg)&0xff00000000000000ull) >> 56) | \
                          (((arg)&0x00ff000000000000ull) >> 40) | \
                          (((arg)&0x0000ff0000000000ull) >> 24) | \
                          (((arg)&0x000000ff00000000ull) >> 8) |  \
                          (((arg)&0x00000000ff000000ull) << 8) |  \
                          (((arg)&0x0000000000ff0000ull) << 24) | \
                          (((arg)&0x000000000000ff00ull) << 40) | \
                          (((arg)&0x00000000000000ffull) << 56))

#endif

namespace ndt
{
enum class eEndian
{
    kLittle,
    kBig,
    kMixed
};

#ifndef HOST_BIG_ENDIAN
constexpr eEndian kEndian = eEndian::kLittle;
constexpr std::string_view kEndianName = "kLittle";
#else
constexpr eEndian kEndian = eEndian::kBig;
constexpr std::string_view kEndianName = "kBig";
#endif

template <typename T = uint32_t>
typename std::enable_if_t<std::is_trivial_v<T> && std::is_standard_layout_v<T>,
                          eEndian>
endian() noexcept
{
    static_assert(sizeof(T) == 4, "sizeof(T) must be 4");
    constexpr uint32_t hostOrder = 0x1020304;
    T result;

    constexpr uint8_t bigOrderBytes[sizeof(T)]{1, 2, 3, 4};
    std::memcpy(&result, &bigOrderBytes, sizeof(T));
    if (hostOrder == result)
    {
        return eEndian::kBig;
    }

    constexpr uint8_t littleOrderBytes[sizeof(T)]{4, 3, 2, 1};
    std::memcpy(&result, &littleOrderBytes, sizeof(T));
    if (hostOrder == result)
    {
        return eEndian::kLittle;
    }
    return eEndian::kMixed;
}

std::string_view endianName() noexcept;

namespace details
{
template <typename T, std::size_t numBytes = sizeof(T)>
class ByteOrder : private NoCopyMoveDefConstructible
{
   public:
    static T toHost(const T aValue) noexcept;
    static T toNet(const T aValue) noexcept;
};

template <>
class ByteOrder<uint8_t, 1> : private NoCopyMoveDefConstructible
{
   public:
    static uint8_t toHost(const uint8_t aValue) noexcept
    {
        return toNet(aValue);
    }
    static uint8_t toNet(const uint8_t aValue) noexcept { return aValue; }
};

template <>
class ByteOrder<uint16_t, 2> : private NoCopyMoveDefConstructible
{
   public:
    static uint16_t toHost(const uint16_t aValue) noexcept
    {
        return toNet(aValue);
    }
    static uint16_t toNet(const uint16_t aValue) noexcept
    {
        if constexpr (kEndian == eEndian::kLittle)
        {
            return swap16(aValue);
        }
        else
        {
            return aValue;
        }
    }
};

template <>
class ByteOrder<uint32_t, 4> : private NoCopyMoveDefConstructible
{
   public:
    static uint32_t toHost(const uint32_t aValue) noexcept
    {
        return toNet(aValue);
    }
    static uint32_t toNet(const uint32_t aValue) noexcept
    {
        if constexpr (kEndian == eEndian::kLittle)
        {
            return swap32(aValue);
        }
        else
        {
            return aValue;
        }
    }
};

template <>
class ByteOrder<uint64_t, 8> : private NoCopyMoveDefConstructible
{
   public:
    static uint64_t toHost(const uint64_t aValue) noexcept
    {
        return toNet(aValue);
    }
    static uint64_t toNet(const uint64_t aValue) noexcept
    {
        if constexpr (kEndian == eEndian::kLittle)
        {
            return swap64(aValue);
        }
        else
        {
            return aValue;
        }
    }
};
}  // namespace details

template <typename T>
T toHost(const T aValue) noexcept
{
    return details::ByteOrder<T>::toHost(aValue);
}

template <typename T>
T toNet(const T aValue) noexcept
{
    return details::ByteOrder<T>::toNet(aValue);
}
}  // namespace ndt

#endif /* ndt_endian_h */