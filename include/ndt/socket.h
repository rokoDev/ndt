#ifndef ndt_socket_h
#define ndt_socket_h

#include <cassert>
#include <functional>

#include "address.h"
#include "buffer.h"
#include "common.h"
#include "exception.h"
#include "nocopyable.h"
#include "utils.h"

namespace ndt
{
class Context;
class ExecutorSelect;
class HandlerSelectBase;

class SocketBase : private Nocopyable
{
    template <typename ImplT>
    friend class ExecutorSelectBase;
    friend class ExecutorSelect;

   public:
    sock_t nativeHandle() const noexcept;
    bool nonBlocking() const noexcept;
    bool isOpen() const noexcept;
    HandlerSelectBase *handler() const noexcept;
    void handler(HandlerSelectBase *aHandler);

    template <typename SFuncsT>
    std::size_t sendTo(const Address &aDst, CBuffer aBuf);

    template <typename SFuncsT>
    std::size_t sendTo(const Address &aDst, CBuffer aBuf, std::error_code &aEc);

    template <typename SFuncsT>
    std::size_t recvFrom(Buffer &aBuf, Address &aSender);

    template <typename SFuncsT>
    std::size_t recvFrom(Buffer &aBuf, Address &aSender, std::error_code &aEc);

    template <typename SFuncsT>
    void close();

    template <typename SFuncsT>
    void close(std::error_code &aEc);

    template <typename SFuncsT>
    void nonBlocking(const bool isNonBlocking);

    template <typename SFuncsT>
    void nonBlocking(const bool isNonBlocking, std::error_code &aEc) noexcept;

   protected:
    ~SocketBase();
    SocketBase() = delete;
    SocketBase(Context &aContext) noexcept;
    SocketBase(SocketBase &&aOther) noexcept;
    SocketBase &operator=(SocketBase &&aOther) noexcept;

    // TODO: rewrite code to get rid of this method
    void setIsSubscribed(bool aIsSubscribed);

    template <typename SFuncsT>
    void open(int socket_family, int socket_type, int protocol,
              std::error_code &aEc);

    template <typename SFuncsT>
    void bind(const uint8_t socket_family, const uint16_t aPort,
              std::error_code &aEc);

    sock_t socketHandle_ = kInvalidSocket;
    bool isOpen_ = false;
    bool isNonBlocking_ = false;
    std::reference_wrapper<Context> context_;
    HandlerSelectBase *handler_ = nullptr;
};

template <typename SFuncsT>
void SocketBase::open(int socket_family, int socket_type, int protocol,
                      std::error_code &aEc)
{
    const auto socketHandle =
        SFuncsT::socket(socket_family, socket_type, protocol);
    if (kInvalidSocket == socketHandle)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
        return;
    }
    socketHandle_ = socketHandle;
    isOpen_ = true;
    setIsSubscribed(true);
}

template <typename SFuncsT>
void SocketBase::bind(const uint8_t socket_family, const uint16_t aPort,
                      std::error_code &aEc)
{
    Address sa(socket_family, aPort);
    const auto result = SFuncsT::bind(socketHandle_, sa.nativeDataConst(),
                                      static_cast<ndt::salen_t>(sa.capacity()));
    if (ndt::kSocketError == result)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
    }
}

template <typename SFuncsT>
std::size_t SocketBase::sendTo(const Address &aDst, CBuffer aBuf)
{
    std::error_code ec;
    const auto bytesSent = SocketBase::sendTo<SFuncsT>(aDst, aBuf, ec);
    throw_if_error(ec);
    return static_cast<std::size_t>(bytesSent);
}

template <typename SFuncsT>
std::size_t SocketBase::sendTo(const Address &aDst, CBuffer aBuf,
                               std::error_code &aEc)
{
    const auto bytesSent = SFuncsT::sendto(
        socketHandle_, aBuf.data(), aBuf.size(), 0, aDst.nativeDataConst(),
        static_cast<ndt::salen_t>(aDst.capacity()));
    if (ndt::kSocketError == bytesSent)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
    }
    return static_cast<std::size_t>(bytesSent);
}

template <typename SFuncsT>
std::size_t SocketBase::recvFrom(Buffer &aBuf, Address &aSender)
{
    std::error_code ec;
    const auto bytesReceived = SocketBase::recvFrom<SFuncsT>(aBuf, aSender, ec);
    throw_if_error(ec);
    return static_cast<std::size_t>(bytesReceived);
}

template <typename SFuncsT>
std::size_t SocketBase::recvFrom(Buffer &aBuf, Address &aSender,
                                 std::error_code &aEc)
{
    ndt::salen_t addrlen = static_cast<ndt::salen_t>(kV6Capacity);
    const auto bytesReceived =
        SFuncsT::recvfrom(socketHandle_, aBuf.data(), aBuf.size(), 0,
                          aSender.nativeData(), &addrlen);
    if (ndt::kSocketError != bytesReceived)
    {
        aBuf.setSize(bytesReceived);
    }
    else
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
    }
    return static_cast<std::size_t>(bytesReceived);
}

template <typename SFuncsT>
void SocketBase::close()
{
    std::error_code ec;
    SocketBase::close<SFuncsT>(ec);
    throw_if_error(ec);
}

template <typename SFuncsT>
void SocketBase::close(std::error_code &aEc)
{
    if (!isOpen_)
    {
        return;
    }
    setIsSubscribed(false);
    const auto result = SFuncsT::close(socketHandle_);
    if (ndt::kSocketError == result)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
        return;
    }
    socketHandle_ = kInvalidSocket;
    isOpen_ = false;
}

template <typename SFuncsT>
void SocketBase::nonBlocking(const bool isNonBlocking)
{
    std::error_code ec;
    SocketBase::nonBlocking<SFuncsT>(isNonBlocking, ec);
    throw_if_error(ec);
}

template <typename SFuncsT>
void SocketBase::nonBlocking(const bool isNonBlocking,
                             std::error_code &aEc) noexcept
{
#if _WIN32
    u_long isNonBlockingArg = isNonBlocking ? 1 : 0;
    const int flags =
        SFuncsT::ioctlsocket(socketHandle_, FIONBIO, &isNonBlockingArg);
#else
    int flags = SFuncsT::fcntl(socketHandle_, F_GETFL, 0);
    if (flags == kSocketError)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
        return;
    }

    flags = isNonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    flags = SFuncsT::fcntl(socketHandle_, F_SETFL, flags);
#endif
    if (flags == kSocketError)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
        return;
    }
    isNonBlocking_ = isNonBlocking;
}

template <typename FlagsT, typename SFuncsT>
class Socket final : public SocketBase
{
    friend class ExecutorSelect;

   public:
    typedef FlagsT SocketT;
    typedef SFuncsT SysCallsT;
    ~Socket();
    Socket() = delete;
    Socket(Socket &&) noexcept;
    Socket &operator=(Socket &&) noexcept;
    Socket(Context &aContext, const FlagsT &flags) noexcept;
    Socket(Context &aContext, const FlagsT &aFlags, uint16_t aPort);

    void open();
    void open(std::error_code &aEc);
    void bind(const uint16_t aPort);
    void bind(const uint16_t aPort, std::error_code &aEc);
    std::size_t sendTo(const Address &aDst, CBuffer aBuf);
    std::size_t sendTo(const Address &aDst, CBuffer aBuf, std::error_code &aEc);
    std::size_t recvFrom(Buffer &aBuf, Address &aSender);
    std::size_t recvFrom(Buffer &aBuf, Address &aSender, std::error_code &aEc);
    void close();
    void close(std::error_code &aEc);
    bool nonBlocking() const noexcept;
    void nonBlocking(const bool isNonBlocking);
    void nonBlocking(const bool isNonBlocking, std::error_code &aEc) noexcept;

    FlagsT flags() const noexcept;

   private:
    FlagsT flags_;
};

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::~Socket()
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(Socket &&aOther) noexcept
    : SocketBase(std::move(aOther)), flags_(std::move(aOther.flags_))
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT> &Socket<FlagsT, SFuncsT>::operator=(
    Socket &&aOther) noexcept
{
    SocketBase::operator=(std::move(aOther));
    std::swap(aOther.flags_, flags_);

    return *this;
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(Context &aContext, const FlagsT &flags) noexcept
    : SocketBase(aContext), flags_(flags)
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(Context &aContext, const FlagsT &aFlags,
                                uint16_t aPort)
    : Socket(aContext, aFlags)
{
    open();
    try
    {
        bind(aPort);
    }
    catch (const std::exception &e)
    {
        close();
        throw;
    }
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::open()
{
    std::error_code ec;
    Socket<FlagsT, SFuncsT>::open(ec);
    throw_if_error(ec);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::open(std::error_code &aEc)
{
    SocketBase::open<SFuncsT>(flags_.sysFamily(), flags_.sysSocketType(),
                              flags_.sysProtocol(), aEc);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::bind(const uint16_t aPort)
{
    std::error_code ec;
    Socket<FlagsT, SFuncsT>::bind(aPort, ec);
    throw_if_error(ec);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::bind(const uint16_t aPort, std::error_code &aEc)
{
    SocketBase::bind<SFuncsT>(flags_.sysFamily(), aPort, aEc);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::sendTo(const Address &aDst, CBuffer aBuf)
{
    return SocketBase::sendTo<SFuncsT>(aDst, aBuf);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::sendTo(const Address &aDst, CBuffer aBuf,
                                            std::error_code &aEc)
{
    return SocketBase::sendTo<SFuncsT>(aDst, aBuf, aEc);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::recvFrom(Buffer &aBuf, Address &aSender)
{
    return SocketBase::recvFrom<SFuncsT>(aBuf, aSender);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::recvFrom(Buffer &aBuf, Address &aSender,
                                              std::error_code &aEc)
{
    return SocketBase::recvFrom<SFuncsT>(aBuf, aSender, aEc);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::close()
{
    SocketBase::close<SFuncsT>();
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::close(std::error_code &aEc)
{
    SocketBase::close<SFuncsT>(aEc);
}

template <typename FlagsT, typename SFuncsT>
bool Socket<FlagsT, SFuncsT>::nonBlocking() const noexcept
{
    return SocketBase::nonBlocking();
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::nonBlocking(const bool isNonBlocking)
{
    SocketBase::nonBlocking<SFuncsT>(isNonBlocking);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::nonBlocking(const bool isNonBlocking,
                                          std::error_code &aEc) noexcept
{
    SocketBase::nonBlocking<SFuncsT>(isNonBlocking, aEc);
}

template <typename FlagsT, typename SFuncsT>
FlagsT Socket<FlagsT, SFuncsT>::flags() const noexcept
{
    return flags_;
}

}  // namespace ndt

#endif /* ndt_socket_h */