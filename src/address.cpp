#include <string>

#include "ndt/address.h"
#include "ndt/exception.h"

namespace ndt
{
Address::~Address() = default;

Address::Address() noexcept { reset(); }

Address::Address(const Address &aOther) noexcept = default;

Address &Address::operator=(Address aOther) noexcept
{
    swap(*this, aOther);
    return *this;
}

Address::Address(Address &&aOther) noexcept { swap(*this, aOther); }

Address::Address(const uint8_t aFamily) : Address(aFamily, 0) {}

Address::Address(const eAddressFamily aFamily) : Address(aFamily, 0) {}

Address::Address(const uint8_t aFamily, const uint16_t aPort) : Address()
{
    addressFamily(aFamily);
    if (aFamily == AF_INET)
    {
        ip(kIPv4Any);
    }
    else
    {
        ip(kIPv6Any);
    }
    port(aPort);
}

Address::Address(const eAddressFamily aFamily, const uint16_t aPort)
    : Address(AddressFamilyUserToSystem.at(aFamily), aPort)
{
}

Address::Address(const ipv4_t &aIPv4, const uint16_t aPort) noexcept : Address()
{
    port(aPort);
    ip(aIPv4);
}

Address::Address(const ipv6_t &aIPv6, const uint16_t aPort) noexcept : Address()
{
    port(aPort);
    ip(aIPv6);
}

Address::Address(const sockaddr &aSockaddr) : Address()
{
    const auto family = aSockaddr.sa_family;

    if (family == AF_INET)
    {
        memcpy(&_sockaddr, &aSockaddr, sizeof(sockaddr_in));
    }
    else if (family == AF_INET6)
    {
        memcpy(&_sockaddr, &aSockaddr, sizeof(sockaddr_in6));
    }
    else
    {
        CheckLogicError(true, exception::kOnlyIPv4OrkIPv6FamilyAllowed);
    }
}

eAddressFamily Address::addressFamily() const noexcept
{
    return AddressFamilySystemToUser.at(
        static_cast<uint8_t>(_sockaddr.sa.sa_family));
}

void Address::addressFamily(const eAddressFamily aFamily)
{
    throwIfFamilyUnspec(aFamily);

    changeFamilyIfDifferent(ndt::AddressFamilyUserToSystem.at(aFamily));
}

uint8_t Address::addressFamilySys() const noexcept
{
    return static_cast<uint8_t>(_sockaddr.sa.sa_family);
}

void Address::addressFamily(const uint8_t aFamily)
{
    const auto family = throwIfUnknownFamily(aFamily);

    throwIfFamilyUnspec(family);

    changeFamilyIfDifferent(aFamily);
}

std::variant<std::monostate, ipv4_t, ipv6_t> Address::ip() const noexcept
{
    const auto kSysFamily = addressFamilySys();

    if (kSysFamily == AF_INET)
    {
        ipv4_t ipValue{ntohl(_sockaddr.sa4.sin_addr.s_addr)};
        return ipValue;
    }

    if (kSysFamily == AF_INET6)
    {
        ipv6_t ipValue{_sockaddr.sa6.sin6_addr};
        return ipValue;
    }

    return std::monostate();
}

void Address::ip(const ipv4_t &aIPv4) noexcept
{
    changeFamilyIfDifferent(AF_INET);
    _sockaddr.sa4.sin_addr.s_addr = htonl(aIPv4.u_ipv4.data32);
}

void Address::ip(const ipv6_t &aIPv6) noexcept
{
    changeFamilyIfDifferent(AF_INET6);
    _sockaddr.sa6.sin6_addr = aIPv6.u_ipv6.nativeData;
}

void Address::port(uint16_t aPort) noexcept
{
    _sockaddr.sa4.sin_port = htons(aPort);
}

uint16_t Address::port() const noexcept
{
    return ntohs(_sockaddr.sa4.sin_port);
}

const sockaddr *Address::nativeDataConst() const noexcept
{
    return &_sockaddr.sa;
}

void Address::reset() noexcept
{
    memset(&_sockaddr, 0, sizeof(_sockaddr));
    _sockaddr.sa.sa_family = AF_UNSPEC;
}

std::size_t Address::capacity() const noexcept
{
    if (addressFamilySys() == AF_INET)
    {
        return capacityV4();
    }
    return capacityV6();
}

std::size_t Address::capacityV4() noexcept { return sizeof(sockaddr_in); }

std::size_t Address::capacityV6() noexcept { return sizeof(sockaddr_in6); }

void swap(Address &aVal1, Address &aVal2) noexcept
{
    std::swap(aVal1._sockaddr, aVal2._sockaddr);
}

bool operator==(const Address &aVal1, const Address &aVal2) noexcept
{
    return (aVal1.capacity() == aVal2.capacity()) &&
           !std::memcmp(aVal1.nativeDataConst(), aVal2.nativeDataConst(),
                        aVal1.capacity());
}

bool operator!=(const Address &aVal1, const Address &aVal2) noexcept
{
    return !(aVal1 == aVal2);
}

void Address::changeFamilyIfDifferent(const uint8_t aFamily)
{
    if (addressFamilySys() != aFamily)
    {
        const auto p = port();
        reset();
        port(p);
        _sockaddr.sa.sa_family = aFamily;
    }
}

void Address::throwIfFamilyUnspec(const eAddressFamily aFamily)
{
    CheckLogicError(aFamily == eAddressFamily::kUnspec,
                    exception::kOnlyIPv4OrkIPv6FamilyAllowed);
}

eAddressFamily Address::throwIfUnknownFamily(const uint8_t aFamily)
{
    const auto foundIt = AddressFamilySystemToUser.find(aFamily);
    const bool isUnknownFamily = foundIt == AddressFamilySystemToUser.end();
    CheckLogicError(isUnknownFamily,
                    exception::kUnknownAddressFamily + std::to_string(aFamily));
    return foundIt->second;
}

sockaddr *Address::nativeData() noexcept { return &_sockaddr.sa; }

}  // namespace ndt
