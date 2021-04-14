#ifndef ndt_utils_h
#define ndt_utils_h

#include <type_traits>
#include <unordered_map>

#include "common.h"
#include "nocopyable.h"
#include "nodefaultconstructible.h"
#include "nomoveable.h"

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

constexpr uint8_t bit_count(const uint64_t aValue)
{
    return (aValue == 0) ? 0 : 1 + bit_count(aValue >> 1);
}

namespace utils
{
bool memvcmp(const void* memptr, unsigned char val, const std::size_t size);

template <typename T>
constexpr std::size_t get_bits_size()
{
    return 8 * sizeof(T);
}

template <>
constexpr std::size_t get_bits_size<bool>()
{
    return 1;
}

template <typename... Ts>
constexpr std::size_t sum_size()
{
    constexpr std::size_t bits_size = (get_bits_size<Ts>() + ...);
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

template <typename T, std::size_t numBytes = sizeof(T),
          class = typename std::enable_if_t<std::is_integral_v<T>>>
class ByteOrder
    : private NoCopyAble
    , private NoMoveAble
    , private NoDefaultConstructible
{
   public:
    static T toHost(const T aValue) noexcept;
    static T toNet(const T aValue) noexcept;
};

template <typename T>
class ByteOrder<T, 1, void>
    : private NoCopyAble
    , private NoMoveAble
    , private NoDefaultConstructible
{
   public:
    static T toHost(const T aValue) noexcept { return aValue; }

    static T toNet(const T aValue) noexcept { return aValue; }
};

template <typename T>
class ByteOrder<T, 2, void>
    : private NoCopyAble
    , private NoMoveAble
    , private NoDefaultConstructible
{
   public:
    static T toHost(const T aValue) noexcept { return ntohs(aValue); }

    static T toNet(const T aValue) noexcept { return htons(aValue); }
};

template <typename T>
class ByteOrder<T, 4, void>
    : private NoCopyAble
    , private NoMoveAble
    , private NoDefaultConstructible
{
   public:
    static T toHost(const T aValue) noexcept { return ntohl(aValue); }

    static T toNet(const T aValue) noexcept { return htonl(aValue); }
};

template <typename T>
class ByteOrder<T, 8, void>
    : private NoCopyAble
    , private NoMoveAble
    , private NoDefaultConstructible
{
   public:
    static T toHost(const T aValue) noexcept { return ntohll(aValue); }

    static T toNet(const T aValue) noexcept { return htonll(aValue); }
};

}  // namespace utils
}  // namespace ndt

#endif /* ndt_utils_h */