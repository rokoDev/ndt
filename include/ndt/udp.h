#ifndef ndt_udp_h
#define ndt_udp_h

#include "utils.h"
#include "socket.h"

namespace ndt
{
template <typename FlagsT, typename SFuncsT>
class Socket;

template <typename FlagsT, typename SFuncsT>
class SocketAddress;

class UDP final
{
   public:
    using Socket = ndt::Socket<UDP, System>;

    static UDP V4() noexcept;
    static UDP V6() noexcept;

    eAddressFamily getFamily() const noexcept;
    eSocketType getSocketType() const noexcept;
    eIPProtocol getProtocol() const noexcept;

    int sysFamily() const noexcept;
    int sysSocketType() const noexcept;
    int sysProtocol() const noexcept;

    friend bool operator==(const UDP& aVal1, const UDP& aVal2);
    friend bool operator!=(const UDP& aVal1, const UDP& aVal2);

   private:
    explicit UDP(const eAddressFamily aAF) noexcept;
    int _af;
};

}  // namespace ndt

#endif /* ndt_udp_h */