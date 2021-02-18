#ifndef ndt_address_h
#define ndt_address_h

#include <string>
#include <variant>

#include "ndt/exception.h"
#include "utils.h"

namespace ndt
{
namespace exception
{
inline const std::string kAddressOnlyIPv4OrkIPv6 =
    "address family can be kIPv4 or kIPv6 only";
inline const std::string kAddressUnknownFamily = "unknown address family: ";
}  // namespace exception

enum class eAddressErrorCode
{
    kSuccess = 0,
    kInvalidAddressFamily,
    kAddressUnknownFamily
};

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
    void changeFamilyIfDifferent(const uint8_t aFamily) noexcept;
    template <typename AF_Type>
    void throwIfInvalidFamily(
        const typename AddressFamily<AF_Type>::type aFamily);
    sockaddr *nativeData() noexcept;
    union
    {
        sockaddr sa;
        sockaddr_in sa4;
        sockaddr_in6 sa6;
    } _sockaddr;
};

template <typename AF_Type>
void Address::throwIfInvalidFamily(
    const typename AddressFamily<AF_Type>::type aFamily)
{
    std::error_code ec;
    utils::validateAddressFamily(aFamily, ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

class AddressErrorCategory : public std::error_category
{
   public:
    virtual const char *name() const noexcept override final;
    virtual std::string message(int c) const override final;
    virtual std::error_condition default_error_condition(
        int c) const noexcept override final;
};

inline std::error_code make_error_code(eAddressErrorCode e)
{
    static AddressErrorCategory c;
    return {static_cast<int>(e), c};
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

#endif /* ndt_address_h */