#ifndef UDP_h
#define UDP_h

#include "NdtUtils.h"
#include "Socket.h"

namespace ndt
{
template <typename FlagsT, typename SFuncsT>
class Socket;

template <typename FlagsT, typename SFuncsT>
class SocketAddress;

class UDP final
{
   public:
    using Socket = ndt::Socket<UDP, details::SocketFuncs>;

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

#endif /* UDP_h */