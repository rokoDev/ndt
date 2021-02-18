#include "ndt/address.h"

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
    const auto family = static_cast<uint8_t>(aSockaddr.sa_family);
    throwIfInvalidFamily<uint8_t>(family);

    if (family == AF_INET)
    {
        memcpy(&_sockaddr, &aSockaddr, sizeof(sockaddr_in));
    }
    else if (family == AF_INET6)
    {
        memcpy(&_sockaddr, &aSockaddr, sizeof(sockaddr_in6));
    }
}

eAddressFamily Address::addressFamily() const noexcept
{
    return AddressFamilySystemToUser.at(
        static_cast<uint8_t>(_sockaddr.sa.sa_family));
}

void Address::addressFamily(const eAddressFamily aFamily)
{
    throwIfInvalidFamily<eAddressFamily>(aFamily);
    changeFamilyIfDifferent(ndt::AddressFamilyUserToSystem.at(aFamily));
}

uint8_t Address::addressFamilySys() const noexcept
{
    return static_cast<uint8_t>(_sockaddr.sa.sa_family);
}

void Address::addressFamily(const uint8_t aFamily)
{
    throwIfInvalidFamily<uint8_t>(aFamily);
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

void Address::changeFamilyIfDifferent(const uint8_t aFamily) noexcept
{
    if (addressFamilySys() != aFamily)
    {
        const auto p = port();
        reset();
        port(p);
        _sockaddr.sa.sa_family = aFamily;
    }
}

sockaddr *Address::nativeData() noexcept { return &_sockaddr.sa; }

const char *AddressErrorCategory::name() const noexcept
{
    return "eAddressErrorCode";
}

std::string AddressErrorCategory::message(int c) const
{
    switch (static_cast<eAddressErrorCode>(c))
    {
        case eAddressErrorCode::kSuccess:
            return "success";
        case eAddressErrorCode::kInvalidAddressFamily:
            return exception::kAddressOnlyIPv4OrkIPv6;
        case eAddressErrorCode::kAddressUnknownFamily:
            return exception::kAddressUnknownFamily;
        default:
            return "unknown";
    }
}

std::error_condition AddressErrorCategory::default_error_condition(
    int c) const noexcept
{
    switch (static_cast<eAddressErrorCode>(c))
    {
        case eAddressErrorCode::kInvalidAddressFamily:
            return make_error_condition(
                std::errc::address_family_not_supported);
        case eAddressErrorCode::kAddressUnknownFamily:
            return make_error_condition(std::errc::invalid_argument);
        default:
            // I have no mapping for this code
            return std::error_condition(c, *this);
    }
}

}  // namespace ndt