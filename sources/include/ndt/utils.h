#ifndef ndt_utils_h
#define ndt_utils_h

#include <cstring>
#include <type_traits>
#include <unordered_map>

#include "common.h"

namespace ndt
{
enum class eSocketType : std::uint8_t
{
    kStream,
    kDgram,
    kRaw
};

enum class eAddressFamily : std::uint8_t
{
    kUnspec,
    kIPv4,
    kIPv6
};

enum class eIPProtocol : std::uint8_t
{
    kIP,
    kTCP,
    kUDP
};

extern const std::unordered_map<eSocketType, int> SocketTypeUserToSystem;
extern const std::unordered_map<int, eSocketType> SocketTypeSystemToUser;

extern const std::unordered_map<eAddressFamily, uint8_t>
    AddressFamilyUserToSystem;
extern const std::unordered_map<uint8_t, eAddressFamily>
    AddressFamilySystemToUser;

extern const std::unordered_map<eIPProtocol, int> IPProtocolUserToSystem;
extern const std::unordered_map<int, eIPProtocol> IPProtocolSystemToUser;

typedef union
{
    uint32_t data32;
    uint16_t data16[2];
    uint8_t data8[4];
    in_addr nativeData;
} u_ipv4_t;

struct ipv4_t
{
    u_ipv4_t u_ipv4;
    friend bool operator==(const ipv4_t& aVal1, const ipv4_t& aVal2) noexcept;
};

typedef union
{
    in6_addr nativeData;
    uint8_t data8[16];
    uint16_t data16[8];
    uint32_t data32[4];
} u_ipv6_t;

struct ipv6_t
{
    u_ipv6_t u_ipv6;
    friend bool operator==(const ipv6_t& aVal1, const ipv6_t& aVal2) noexcept;
};

extern const ipv4_t kIPv4Any;
extern const ipv4_t kIPv4Loopback;
extern const ipv6_t kIPv6Any;
extern const ipv6_t kIPv6Loopback;

inline constexpr std::size_t kV4Capacity = sizeof(sockaddr_in);
inline constexpr std::size_t kV6Capacity = sizeof(sockaddr_in6);

namespace utils
{
bool memvcmp(const void* memptr, unsigned char val, const std::size_t size);

constexpr uint8_t bits_count(const uint64_t aValue)
{
    return (aValue == 0) ? 0 : 1 + bits_count(aValue >> 1);
}

template <uint8_t BitsCount>
struct uint_from_nbits
{
   private:
    static constexpr auto getType()
    {
        if constexpr ((BitsCount > 0) && (BitsCount <= 8))
        {
            return uint8_t();
        }
        else if constexpr ((BitsCount > 8) && (BitsCount <= 16))
        {
            return uint16_t();
        }
        else if constexpr ((BitsCount > 16) && (BitsCount <= 32))
        {
            return uint32_t();
        }
        else if constexpr ((BitsCount > 32) && (BitsCount <= 64))
        {
            return uint64_t();
        }
        else
        {
            static_assert(BitsCount < sizeof(uint64_t) * 8,
                          "BitsCount count can't be more than 64");
            return uint8_t();
        }
    }

   public:
    using type = decltype(getType());
};

template <uint8_t BitsCount>
using uint_from_nbits_t = typename uint_from_nbits<BitsCount>::type;

template <typename T>
struct enum_properties
{
    static constexpr uint8_t numBits =
        bits_count(static_cast<uint64_t>(T::Count));
    using SerializeT = uint_from_nbits_t<numBits>;
};

template <typename T>
constexpr std::size_t num_bits()
{
    return 8 * sizeof(T);
}

template <>
constexpr std::size_t num_bits<bool>()
{
    return 1;
}

template <typename... Ts>
constexpr std::size_t sum_size()
{
    constexpr std::size_t bits_size = (num_bits<Ts>() + ...);
    if constexpr (bits_size % 8 > 0)
    {
        return bits_size / 8 + 1;
    }
    else
    {
        return bits_size / 8;
    }
}

template <typename F, typename... Ts>
void for_each_arg(F&& f, Ts&&... ts)
{
    using I = std::initializer_list<int>;
    (void)I{(std::forward<F>(f)(std::forward<Ts>(ts)), 0)...};
}

template <typename F, typename T>
void for_each_in_tuple(F&& f, T&& t)
{
    std::apply(
        [&f](auto&&... xs) {
            for_each_arg(f, std::forward<decltype(xs)>(xs)...);
        },
        std::forward<T>(t));
}

template <class To, class From>
typename std::enable_if_t<sizeof(To) == sizeof(From) &&
                              std::is_trivially_copyable_v<From> &&
                              std::is_trivially_copyable_v<To>,
                          To>
bit_cast(const From& src) noexcept
{
    static_assert(std::is_trivially_constructible_v<To>,
                  "This implementation additionally requires destination type "
                  "to be trivially constructible");

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<typename std::underlying_type_t<E>>(e);
}

}  // namespace utils
}  // namespace ndt

#endif /* ndt_utils_h */