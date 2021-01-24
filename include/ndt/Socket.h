#ifndef Socket_h
#define Socket_h

#include <cstddef>

#include "Address.h"
#include "CommonInclude.h"
#include "NdtException.h"
#include "NdtUtils.h"

namespace ndt
{
namespace details
{
class SocketFuncs final
{
    ~SocketFuncs() = delete;
    SocketFuncs() = delete;
    SocketFuncs(const SocketFuncs &) = delete;
    SocketFuncs &operator=(const SocketFuncs &) = delete;
    SocketFuncs(SocketFuncs &&) = delete;
    SocketFuncs &operator=(SocketFuncs &&) = delete;

   public:
    static int bind(sock_t sockfd, const struct sockaddr *addr,
                    ndt::salen_t addrlen);
    static sdlen_t recvfrom(sock_t sockfd, ndt::buf_t buf, ndt::dlen_t len,
                            int flags, struct sockaddr *src_addr,
                            ndt::salen_t *addrlen);
    static sdlen_t sendto(sock_t sockfd, ndt::cbuf_t buf, dlen_t len, int flags,
                          const struct sockaddr *dest_addr,
                          ndt::salen_t addrlen);
    static sock_t socket(int socket_family, int socket_type, int protocol);
    static int close(sock_t fd);
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
    std::size_t sendTo(const Address &aDst, ndt::cbuf_t aDataPtr,
                       const dlen_t aDataSize);
    std::size_t recvFrom(ndt::buf_t aDataPtr, ndt::dlen_t aDataSize,
                         Address &aSender);
    void close();

    const FlagsT &flags() const noexcept;

   protected:
    sock_t _socketHandle;
    FlagsT _flags;
    bool _isOpen = false;
};

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::~Socket()
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(Socket &&aOther) noexcept
    : _socketHandle(std::exchange(aOther._socketHandle, kInvalidSocket))
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
    : _socketHandle(kInvalidSocket), _flags(flags), _isOpen(false)
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
    CheckRuntimeError(kInvalidSocket == _socketHandle,
                      ndt::exception::kSocketOpen);
    _isOpen = true;
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::bind(const uint16_t aPort)
{
    CheckLogicError(!isOpen(), exception::kSocketMustBeOpenToBind);
    Address sa(_flags.getFamily(), aPort);
    const auto result = SFuncsT::bind(_socketHandle, sa.nativeDataConst(),
                                      static_cast<ndt::salen_t>(sa.capacity()));
    CheckRuntimeError(ndt::kSocketError == result, ndt::exception::kSocketBind);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::sendTo(const Address &aDst,
                                            ndt::cbuf_t aDataPtr,
                                            const ndt::dlen_t aLen)
{
    const auto bytesSent = SFuncsT::sendto(
        _socketHandle, aDataPtr, aLen, 0, aDst.nativeDataConst(),
        static_cast<ndt::salen_t>(aDst.capacity()));
    CheckRuntimeError(ndt::kSocketError == bytesSent,
                      ndt::exception::kSocketSendTo);
    return static_cast<std::size_t>(bytesSent);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::recvFrom(ndt::buf_t aDataPtr,
                                              ndt::dlen_t aDataSize,
                                              Address &aSender)
{
    ndt::salen_t addrlen = static_cast<ndt::salen_t>(aSender.capacityV6());
    const auto result = SFuncsT::recvfrom(_socketHandle, aDataPtr, aDataSize, 0,
                                          aSender.nativeData(), &addrlen);
    CheckRuntimeError(ndt::kSocketError == result,
                      ndt::exception::kSocketRecvFrom);

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
    CheckRuntimeError(ndt::kSocketError == result,
                      ndt::exception::kSocketClose);
    _socketHandle = kInvalidSocket;
    _isOpen = false;
}

template <typename FlagsT, typename SFuncsT>
const FlagsT &Socket<FlagsT, SFuncsT>::flags() const noexcept
{
    return _flags;
}

}  // namespace ndt

#endif /* Socket_h */