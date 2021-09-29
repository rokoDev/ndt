#ifndef ndt_endian_h
#define ndt_endian_h

#include <climits>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>

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
template <typename T, std::size_t... I>
constexpr T bswap_impl(T aValue, std::index_sequence<I...>)
{
    return (... | static_cast<T>((0b11111111 & (aValue >> (I * CHAR_BIT)))
                                 << ((sizeof(T) - 1 - I) * CHAR_BIT)));
};
}  // namespace details

template <typename T, typename U = typename std::make_unsigned<T>::type>
constexpr U bswap(T aValue)
{
    return details::bswap_impl<U>(aValue,
                                  std::make_index_sequence<sizeof(T)>{});
}

template <typename T>
constexpr T toNet(const T aValue) noexcept
{
    if constexpr (kEndian == eEndian::kLittle)
    {
        return bswap(aValue);
    }
    else
    {
        return aValue;
    }
}

template <typename T>
constexpr T toHost(const T aValue) noexcept
{
    return toNet(aValue);
}
}  // namespace ndt

#endif /* ndt_endian_h */