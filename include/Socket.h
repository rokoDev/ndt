#ifndef Socket_h
#define Socket_h

#include <unistd.h>

#include <cstddef>

#include "Address.h"
#include "CommonInclude.h"
#include "NetException.h"
#include "NetUtils.h"

namespace net
{
namespace details
{
using socket_t = decltype(socket(0, 0, 0));
inline socket_t kInvalidSocket = -1;
inline socket_t kClosedSocket = -2;

class SocketFuncs final
{
    ~SocketFuncs() = delete;
    SocketFuncs() = delete;
    SocketFuncs(const SocketFuncs &) = delete;
    SocketFuncs &operator=(const SocketFuncs &) = delete;
    SocketFuncs(SocketFuncs &&) = delete;
    SocketFuncs &operator=(SocketFuncs &&) = delete;

   public:
    static int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    static ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                            struct sockaddr *src_addr, socklen_t *addrlen);
    static ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                          const struct sockaddr *dest_addr, socklen_t addrlen);
    static int socket(int socket_family, int socket_type, int protocol);
    static int close(int fd);
};

}  // namespace details

template <typename FlagsT, typename SFuncsT>
class Socket
{
   public:
    ~Socket();
    Socket() = delete;
    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;
    Socket(Socket &&) noexcept;
    Socket &operator=(Socket &&) noexcept;
    explicit Socket(const FlagsT &flags) noexcept;
    Socket(const FlagsT &aFlags, uint16_t aPort);

    bool isOpen() const noexcept;
    void open();
    void bind(const uint16_t aPort);
    std::size_t sendTo(const Address &aDst, const char *aDataPtr,
                       const std::size_t aDataSize);
    std::size_t recvFrom(char *aDataPtr, std::size_t aDataSize,
                         Address &aSender);
    void close();

    const FlagsT &flags() const noexcept;

   protected:
    details::socket_t _socketHandle;
    FlagsT _flags;
    bool _isOpen = false;
};

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::~Socket()
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(Socket &&aOther) noexcept
    : _socketHandle(std::exchange(aOther._socketHandle, details::kClosedSocket))
    , _flags(std::move(aOther._flags))
    , _isOpen(std::exchange(aOther._isOpen, false))
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT> &Socket<FlagsT, SFuncsT>::operator=(
    Socket &&aOther) noexcept
{
    std::swap(aOther._socketHandle, _socketHandle);
    std::swap(aOther._flags, _flags);
    std::swap(aOther._isOpen, _isOpen);

    return *this;
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(const FlagsT &flags) noexcept
    : _socketHandle(details::kClosedSocket), _flags(flags), _isOpen(false)
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(const FlagsT &aFlags, uint16_t aPort)
    : Socket(aFlags)
{
    open();
    bind(aPort);
}

template <typename FlagsT, typename SFuncsT>
bool Socket<FlagsT, SFuncsT>::isOpen() const noexcept
{
    return _isOpen;
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::open()
{
    CheckLogicError(isOpen(), exception::kSocketAlreadyOpened);
    _socketHandle = SFuncsT::socket(_flags.sysFamily(), _flags.sysSocketType(),
                                    _flags.sysProtocol());
    CheckRuntimeError(details::kInvalidSocket == _socketHandle,
                      net::exception::kSocketOpen);
    _isOpen = true;
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::bind(const uint16_t aPort)
{
    CheckLogicError(!isOpen(), exception::kSocketMustBeOpenToBind);
    Address sa(_flags.getFamily(), aPort);
    const auto result = SFuncsT::bind(_socketHandle, sa.nativeDataConst(),
                                      static_cast<socklen_t>(sa.capacity()));
    CheckRuntimeError(net::OPERATION_FAILED == result,
                      net::exception::kSocketBind);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::sendTo(const Address &aDst,
                                            const char *aDataPtr,
                                            const std::size_t aLen)
{
    const auto bytesSent = SFuncsT::sendto(
        _socketHandle, aDataPtr, aLen, 0, aDst.nativeDataConst(),
        static_cast<socklen_t>(aDst.capacity()));
    CheckRuntimeError(net::OPERATION_FAILED == bytesSent,
                      net::exception::kSocketSendTo);
    return static_cast<std::size_t>(bytesSent);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::recvFrom(char *aDataPtr,
                                              std::size_t aDataSize,
                                              Address &aSender)
{
    socklen_t addrlen = static_cast<socklen_t>(aSender.capacityV6());
    const auto result = SFuncsT::recvfrom(_socketHandle, aDataPtr, aDataSize, 0,
                                          aSender.nativeData(), &addrlen);
    CheckRuntimeError(net::OPERATION_FAILED == result,
                      net::exception::kSocketRecvFrom);

    return static_cast<std::size_t>(result);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::close()
{
    if (!_isOpen)
    {
        return;
    }
    const auto result = SFuncsT::close(_socketHandle);
    CheckRuntimeError(net::OPERATION_FAILED == result,
                      net::exception::kSocketClose);
    _socketHandle = details::kClosedSocket;
    _isOpen = false;
}

template <typename FlagsT, typename SFuncsT>
const FlagsT &Socket<FlagsT, SFuncsT>::flags() const noexcept
{
    return _flags;
}

}  // namespace net

#endif /* Socket_h */