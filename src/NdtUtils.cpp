#include "ndt/NdtUtils.h"

#include <fmt/core.h>
#include <fmt/format.h>

#include <cstring>
#include <string>

#include "ndt/CommonInclude.h"

namespace ndt
{
const std::unordered_map<eSocketType, int> SocketTypeUserToSystem = {
    {eSocketType::kStream, SOCK_STREAM},
    {eSocketType::kDgram, SOCK_DGRAM},
    {eSocketType::kRaw, SOCK_RAW}};

const std::unordered_map<int, eSocketType> SocketTypeSystemToUser = {
    {SOCK_STREAM, eSocketType::kStream},
    {SOCK_DGRAM, eSocketType::kDgram},
    {
        SOCK_RAW,
        eSocketType::kRaw,
    }};

const std::unordered_map<eAddressFamily, uint8_t> AddressFamilyUserToSystem = {
    {eAddressFamily::kUnspec, AF_UNSPEC},
    {eAddressFamily::kIPv4, AF_INET},
    {eAddressFamily::kIPv6, AF_INET6}};

const std::unordered_map<uint8_t, eAddressFamily> AddressFamilySystemToUser = {
    {AF_UNSPEC, eAddressFamily::kUnspec},
    {AF_INET, eAddressFamily::kIPv4},
    {AF_INET6, eAddressFamily::kIPv6}};

const std::unordered_map<eIPProtocol, int> IPProtocolUserToSystem = {
    {eIPProtocol::kIP, IPPROTO_IP},
    {eIPProtocol::kTCP, IPPROTO_TCP},
    {eIPProtocol::kUDP, IPPROTO_UDP}};

const std::unordered_map<int, eIPProtocol> IPProtocolSystemToUser = {
    {IPPROTO_IP, eIPProtocol::kIP},
    {IPPROTO_TCP, eIPProtocol::kTCP},
    {IPPROTO_UDP, eIPProtocol::kUDP}};

bool operator==(const ipv4_t &aVal1, const ipv4_t &aVal2) noexcept
{
    return aVal1.u_ipv4.data32 == aVal2.u_ipv4.data32;
}

bool operator==(const ipv6_t &aVal1, const ipv6_t &aVal2) noexcept
{
    return (aVal1.u_ipv6.data32[0] == aVal2.u_ipv6.data32[0]) &&
           (aVal1.u_ipv6.data32[1] == aVal2.u_ipv6.data32[1]) &&
           (aVal1.u_ipv6.data32[2] == aVal2.u_ipv6.data32[2]) &&
           (aVal1.u_ipv6.data32[3] == aVal2.u_ipv6.data32[3]);
}

const ipv4_t kIPv4Any{INADDR_ANY};
const ipv4_t kIPv4Loopback{INADDR_LOOPBACK};

const ipv6_t kIPv6Any{u_ipv6_t{in6addr_any}};
const ipv6_t kIPv6Loopback{u_ipv6_t{in6addr_loopback}};

namespace utils
{
bool memvcmp(const void *memptr, unsigned char val, const std::size_t size)
{
    if ((0 == size) || (nullptr == memptr))
    {
        return false;
    }
    const unsigned char *mm = static_cast<const unsigned char *>(memptr);
    return (*mm == val) && (memcmp(mm, mm + 1, size - 1) == 0);
}
}  // namespace utils

}  // namespace ndt
