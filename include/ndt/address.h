#ifndef ndt_address_h
#define ndt_address_h

#include <algorithm>
#include <variant>

#include "utils.h"

namespace ndt
{
class Address final
{
    template <typename FlagsT, typename SFuncsT>
    friend class Socket;

   public:
    ~Address();
    Address() noexcept;

    Address(const Address &aOther) noexcept;
    Address &operator=(Address aOther) noexcept;

    Address(Address &&aOther) noexcept;

    explicit Address(const uint8_t aFamily);
    explicit Address(const eAddressFamily aFamily);

    Address(const uint8_t aFamily, const uint16_t aPort);
    Address(const eAddressFamily aFamily, const uint16_t aPort);
    Address(const ipv4_t &aIPv4, const uint16_t aPort) noexcept;
    Address(const ipv6_t &aIPv6, const uint16_t aPort) noexcept;

    explicit Address(const sockaddr &aSockaddr);

    eAddressFamily addressFamily() const noexcept;
    void addressFamily(const eAddressFamily aFamily);

    uint8_t addressFamilySys() const noexcept;
    void addressFamily(const uint8_t aFamily);

    std::variant<std::monostate, ipv4_t, ipv6_t> ip() const noexcept;
    void ip(const ipv4_t &aIPv4) noexcept;
    void ip(const ipv6_t &aIPv6) noexcept;

    void port(uint16_t aPort) noexcept;
    uint16_t port() const noexcept;

    const sockaddr *nativeDataConst() const noexcept;

    void reset() noexcept;

    std::size_t capacity() const noexcept;
    static std::size_t capacityV4() noexcept;
    static std::size_t capacityV6() noexcept;

    friend void swap(Address &aVal1, Address &aVal2) noexcept;
    friend bool operator==(const Address &aVal1, const Address &aVal2) noexcept;
    friend bool operator!=(const Address &aVal1, const Address &aVal2) noexcept;

   private:
    void changeFamilyIfDifferent(const uint8_t aFamily);
    void throwIfFamilyUnspec(const eAddressFamily aFamily);
    eAddressFamily throwIfUnknownFamily(const uint8_t aFamily);
    sockaddr *nativeData() noexcept;
    union
    {
        sockaddr sa;
        sockaddr_in sa4;
        sockaddr_in6 sa6;
    } _sockaddr;
};

}  // namespace ndt

#endif /* ndt_address_h */
