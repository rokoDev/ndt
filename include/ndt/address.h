#ifndef ndt_address_h
#define ndt_address_h

#include <string>
#include <variant>

#include "buffer.h"
#include "exception.h"
#include "system_wrappers.h"
#include "utils.h"

namespace ndt
{
inline const char *kAddressErrorCategoryCStr = "ndt_address_ec";
inline const std::string kInvalidAddressFamilyDescr =
    "address family can be kIPv4 or kIPv6 only";
inline const std::string kAddressUnknownFamilyDescr = "unknown address family";
inline const std::string kStringIsNotIpAddressDescr =
    "string is not valid ip address";

enum class eAddressErrorCode
{
    kSuccess = 0,
    kInvalidAddressFamily,
    kAddressUnknownFamilyDescr,
    kStringIsNotIpAddress
};

class AddressErrorCategory : public std::error_category
{
   public:
    virtual const char *name() const noexcept override final;
    virtual std::string message(int c) const override final;
    virtual std::error_condition default_error_condition(
        int c) const noexcept override final;
};

inline const AddressErrorCategory &getAddressErrorCategory()
{
    static AddressErrorCategory c;
    return c;
}

inline std::error_code make_error_code(eAddressErrorCode e)
{
    return {static_cast<int>(e), getAddressErrorCategory()};
}

}  // namespace ndt

namespace std
{
// Tell the C++ 11 STL metaprogramming that enum ndt::eAddressErrorCode
// is registered with the standard error code system
template <>
struct is_error_code_enum<ndt::eAddressErrorCode> : true_type
{
};
}  // namespace std

namespace ndt
{
template <typename T>
struct AddressFamily
{
};
template <>
struct AddressFamily<uint8_t>
{
    using type = uint8_t;
};
template <>
struct AddressFamily<eAddressFamily>
{
    using type = eAddressFamily;
};

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
    template <typename SysWrapperT = System>
    void ipV4(const char *aIPCStr, std::error_code &aEc) noexcept;

    template <typename SysWrapperT = System>
    void ipV4(const char *aIPCStr);

    template <typename SysWrapperT = System>
    void ipV6(const char *aIPCStr, std::error_code &aEc) noexcept;

    template <typename SysWrapperT = System>
    void ipV6(const char *aIPCStr);

    template <typename SysWrapperT = System>
    void ip(const char *aIPCStr, std::error_code &aEc) noexcept;

    template <typename SysWrapperT = System>
    void ip(const char *aIPCStr);

    void port(uint16_t aPort) noexcept;
    uint16_t port() const noexcept;

    const sockaddr *nativeDataConst() const noexcept;
    void reset() noexcept;
    std::size_t capacity() const noexcept;

    template <typename SysWrapperT = System>
    void ipStr(buffer aBuf) const;
    template <typename SysWrapperT = System>
    void ipStr(buffer aBuf, std::error_code &aEc) const noexcept;

    static void validateAddressFamily(const eAddressFamily aAddressFamily,
                                      std::error_code &aEc) noexcept;
    static void validateAddressFamily(const uint8_t aAddressFamily,
                                      std::error_code &aEc) noexcept;

    friend void swap(Address &aVal1, Address &aVal2) noexcept;
    friend bool operator==(const Address &aVal1, const Address &aVal2) noexcept;
    friend bool operator!=(const Address &aVal1, const Address &aVal2) noexcept;

   private:
    inline void setFamily(const uint8_t aFamily) noexcept;
    const void *ipPtr() const noexcept;
    template <typename AF_Type>
    void throwIfInvalidFamily(
        const typename AddressFamily<AF_Type>::type aFamily);
    sockaddr *nativeData() noexcept;
    union
    {
        sockaddr sa;
        sockaddr_in sa4;
        sockaddr_in6 sa6;
    } sockaddr_;
};

template <typename AF_Type>
void Address::throwIfInvalidFamily(
    const typename AddressFamily<AF_Type>::type aFamily)
{
    std::error_code ec;
    validateAddressFamily(aFamily, ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

template <typename SysWrapperT>
void Address::ipV4(const char *aIPCStr, std::error_code &aEc) noexcept
{
    const int result = SysWrapperT::inet_pton(
        AF_INET, aIPCStr, static_cast<void *>(&sockaddr_.sa4.sin_addr.s_addr));
    if (result == 1)
    {
        addressFamily(AF_INET);
    }
    else if (result == 0)
    {
        aEc = eAddressErrorCode::kStringIsNotIpAddress;
    }
    else
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
    }
}

template <typename SysWrapperT>
void Address::ipV4(const char *aIPCStr)
{
    std::error_code ec;
    ipV4<SysWrapperT>(aIPCStr, ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

template <typename SysWrapperT>
void Address::ipV6(const char *aIPCStr, std::error_code &aEc) noexcept
{
    const int result = SysWrapperT::inet_pton(
        AF_INET6, aIPCStr, static_cast<void *>(&sockaddr_.sa6.sin6_addr));
    if (result == 1)
    {
        addressFamily(AF_INET6);
    }
    else if (result == 0)
    {
        aEc = eAddressErrorCode::kStringIsNotIpAddress;
    }
    else
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
    }
}

template <typename SysWrapperT>
void Address::ipV6(const char *aIPCStr)
{
    std::error_code ec;
    ipV6<SysWrapperT>(aIPCStr, ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

template <typename SysWrapperT>
void Address::ip(const char *aIPCStr, std::error_code &aEc) noexcept
{
    ipV4<SysWrapperT>(aIPCStr, aEc);
    if (aEc)
    {
        aEc.clear();
        ipV6<SysWrapperT>(aIPCStr, aEc);
    }
}

template <typename SysWrapperT>
void Address::ip(const char *aIPCStr)
{
    std::error_code ec;
    ip<SysWrapperT>(aIPCStr, ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

template <typename SysWrapperT>
void Address::ipStr(buffer aBuf) const
{
    std::error_code ec;
    ipStr<SysWrapperT>(aBuf, ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

template <typename SysWrapperT>
void Address::ipStr(buffer aBuf, std::error_code &aEc) const noexcept
{
    if (const auto ipPointer = ipPtr(); ipPointer)
    {
        const char *result = SysWrapperT::inet_ntop(
            addressFamilySys(), ipPointer, aBuf.charPtr(),
            static_cast<salen_t>(aBuf.size()));
        if (!result)
        {
            aEc.assign(systemErrorCodeGetter(), std::system_category());
        }
    }
    else
    {
        aEc = eAddressErrorCode::kInvalidAddressFamily;
    }
}

}  // namespace ndt

#endif /* ndt_address_h */