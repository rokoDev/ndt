#include "ndt/udp.h"
#include "ndt/common.h"

namespace ndt
{
UDP UDP::V4() noexcept { return UDP(eAddressFamily::kIPv4); }

UDP UDP::V6() noexcept { return UDP(eAddressFamily::kIPv6); }

eAddressFamily UDP::getFamily() const noexcept
{
    return (_af == AF_INET) ? eAddressFamily::kIPv4 : eAddressFamily::kIPv6;
}

eSocketType UDP::getSocketType() const noexcept { return eSocketType::kDgram; }

eIPProtocol UDP::getProtocol() const noexcept { return eIPProtocol::kUDP; }

bool operator==(const UDP& aVal1, const UDP& aVal2)
{
    return aVal1._af == aVal2._af;
}

bool operator!=(const UDP& aVal1, const UDP& aVal2)
{
    return aVal1._af != aVal2._af;
}

UDP::UDP(const eAddressFamily aAF) noexcept
    : _af((aAF == eAddressFamily::kIPv4) ? AF_INET : AF_INET6)
{
}

int UDP::sysFamily() const noexcept { return _af; }

int UDP::sysSocketType() const noexcept { return SOCK_DGRAM; }

int UDP::sysProtocol() const noexcept { return IPPROTO_UDP; }

}  // namespace ndt
