#ifndef ndt_endian_h
#define ndt_endian_h

#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>

namespace ndt
{
enum class eEndian
{
    kLittle,
    kBig,
    kMixed
};

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

std::string_view endianName() noexcept
{
    using namespace std::literals;
    if (endian() == eEndian::kLittle)
    {
        return "kLittle"sv;
    }
    else if (endian() == eEndian::kBig)
    {
        return "kBig"sv;
    }
    else
    {
        return "kMixed"sv;
    }
}
}  // namespace ndt

#endif /* ndt_endian_h */