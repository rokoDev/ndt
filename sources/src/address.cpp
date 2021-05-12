#include "ndt/address.h"

#include <cstring>

#include "ndt/bin_rw.h"

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
    throwIfInvalidFamily<uint8_t>(aFamily);
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
        std::memcpy(&sockaddr_, &aSockaddr, sizeof(sockaddr_in));
    }
    else if (family == AF_INET6)
    {
        std::memcpy(&sockaddr_, &aSockaddr, sizeof(sockaddr_in6));
    }
}

eAddressFamily Address::addressFamily() const noexcept
{
    return AddressFamilySystemToUser.at(
        static_cast<uint8_t>(sockaddr_.sa.sa_family));
}

void Address::addressFamily(const eAddressFamily aFamily)
{
    throwIfInvalidFamily<eAddressFamily>(aFamily);
    setFamily(ndt::AddressFamilyUserToSystem.at(aFamily));
}

uint8_t Address::addressFamilySys() const noexcept
{
    return static_cast<uint8_t>(sockaddr_.sa.sa_family);
}

void Address::addressFamily(const uint8_t aFamily)
{
    throwIfInvalidFamily<uint8_t>(aFamily);
    setFamily(aFamily);
}

std::variant<std::monostate, ipv4_t, ipv6_t> Address::ip() const noexcept
{
    const auto kSysFamily = addressFamilySys();

    if (kSysFamily == AF_INET)
    {
        ipv4_t ipValue{ntohl(sockaddr_.sa4.sin_addr.s_addr)};
        return ipValue;
    }

    if (kSysFamily == AF_INET6)
    {
        ipv6_t ipValue{sockaddr_.sa6.sin6_addr};
        return ipValue;
    }

    return std::monostate();
}

void Address::ip(const ipv4_t &aIPv4) noexcept
{
    setFamily(AF_INET);
    sockaddr_.sa4.sin_addr.s_addr = htonl(aIPv4.u_ipv4.data32);
}

void Address::ip(const ipv6_t &aIPv6) noexcept
{
    setFamily(AF_INET6);
    sockaddr_.sa6.sin6_addr = aIPv6.u_ipv6.nativeData;
}

void Address::port(uint16_t aPort) noexcept
{
    sockaddr_.sa4.sin_port = htons(aPort);
}

uint16_t Address::port() const noexcept
{
    return ntohs(sockaddr_.sa4.sin_port);
}

const sockaddr *Address::nativeDataConst() const noexcept
{
    return &sockaddr_.sa;
}

void Address::reset() noexcept
{
    memset(&sockaddr_, 0, sizeof(sockaddr_));
    sockaddr_.sa.sa_family = AF_UNSPEC;
}

std::size_t Address::capacity() const noexcept
{
    if (addressFamilySys() == AF_INET)
    {
        return kV4Capacity;
    }
    return kV6Capacity;
}

void Address::validateAddressFamily(const eAddressFamily aAddressFamily,
                                    std::error_code &aEc) noexcept
{
    if ((aAddressFamily == eAddressFamily::kIPv4) ||
        (aAddressFamily == eAddressFamily::kIPv6))
    {
        return;
    }
    aEc = eAddressErrorCode::kInvalidAddressFamily;
}

void Address::validateAddressFamily(const uint8_t aAddressFamily,
                                    std::error_code &aEc) noexcept
{
    if ((aAddressFamily == AF_INET) || (aAddressFamily == AF_INET6))
    {
        return;
    }
    if (aAddressFamily == AF_UNSPEC)
    {
        aEc = eAddressErrorCode::kInvalidAddressFamily;
    }
    else
    {
        aEc = eAddressErrorCode::kAddressUnknownFamilyDescr;
    }
}

void swap(Address &aVal1, Address &aVal2) noexcept
{
    std::swap(aVal1.sockaddr_, aVal2.sockaddr_);
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

void Address::setFamily(const uint8_t aFamily) noexcept
{
    sockaddr_.sa.sa_family = aFamily;
}

void const *Address::ipPtr() const noexcept
{
    if (const auto kAf = addressFamilySys(); kAf == AF_INET)
    {
        return static_cast<const void *>(&sockaddr_.sa4.sin_addr);
    }
    else if (kAf == AF_INET6)
    {
        return static_cast<const void *>(&sockaddr_.sa6.sin6_addr);
    }
    else
    {
        return nullptr;
    }
}

sockaddr *Address::nativeData() noexcept { return &sockaddr_.sa; }

const char *AddressErrorCategory::name() const noexcept
{
    return kAddressErrorCategoryCStr;
}

std::string AddressErrorCategory::message(int c) const
{
    switch (static_cast<eAddressErrorCode>(c))
    {
        case eAddressErrorCode::kSuccess:
            return "success";
        case eAddressErrorCode::kInvalidAddressFamily:
            return kInvalidAddressFamilyDescr;
        case eAddressErrorCode::kAddressUnknownFamilyDescr:
            return kAddressUnknownFamilyDescr;
        case eAddressErrorCode::kStringIsNotIpAddress:
            return kStringIsNotIpAddressDescr;
        default:
            return "unknown";
    }
}

std::error_condition AddressErrorCategory::default_error_condition(
    int c) const noexcept
{
    return std::error_condition(c, *this);
}

}  // namespace ndt