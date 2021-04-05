#ifndef ndt_socket_h
#define ndt_socket_h

#include <cassert>
#include <functional>

#include "address.h"
#include "buffer.h"
#include "common.h"
#include "exception.h"
#include "useful_base_types.h"
#include "utils.h"

namespace ndt
{
template <typename SysWrapperT>
class Context;

template <typename SysWrapperT>
class ExecutorSelect;

template <typename SysWrapperT>
class HandlerSelectBase;

template <typename SysWrapperT>
class SocketBase : private NoCopyAble
{
    template <typename ImplT, typename SysWrappersT>
    friend class ExecutorSelectBase;

    template <typename SysWrappersT>
    friend class ExecutorSelect;

   public:
    sock_t nativeHandle() const noexcept;
    bool nonBlocking() const noexcept;
    bool isOpen() const noexcept;
    HandlerSelectBase<SysWrapperT> *handler() const noexcept;
    void handler(HandlerSelectBase<SysWrapperT> *aHandler);

    std::size_t sendTo(const Address &aDst, CBuffer aBuf);
    std::size_t sendTo(const Address &aDst, CBuffer aBuf, std::error_code &aEc);
    std::size_t recvFrom(Buffer &aBuf, Address &aSender);
    std::size_t recvFrom(Buffer &aBuf, Address &aSender, std::error_code &aEc);
    void close();
    void close(std::error_code &aEc);
    void nonBlocking(const bool isNonBlocking);
    void nonBlocking(const bool isNonBlocking, std::error_code &aEc) noexcept;

   protected:
    ~SocketBase();
    SocketBase() = delete;
    SocketBase(Context<SysWrapperT> &aContext) noexcept;
    SocketBase(SocketBase &&aOther) noexcept;
    SocketBase &operator=(SocketBase &&aOther) noexcept;

    // TODO: rewrite code to get rid of this method
    void setIsSubscribed(bool aIsSubscribed);

    void open(int socket_family, int socket_type, int protocol,
              std::error_code &aEc);

    void bind(const uint8_t socket_family, const uint16_t aPort,
              std::error_code &aEc);

    sock_t socketHandle_ = kInvalidSocket;
    bool isOpen_ = false;
    bool isNonBlocking_ = false;
    std::reference_wrapper<Context<SysWrapperT>> context_;
    HandlerSelectBase<SysWrapperT> *handler_ = nullptr;
};

template <typename SysWrapperT>
SocketBase<SysWrapperT>::~SocketBase()
{
    assert(!isOpen_ && "Error: socket must be closed before destruction");
}

template <typename SysWrapperT>
SocketBase<SysWrapperT>::SocketBase(Context<SysWrapperT> &aContext) noexcept
    : socketHandle_(kInvalidSocket)
    , isOpen_(false)
    , isNonBlocking_(false)
    , context_(aContext)
    , handler_(nullptr)
{
}

template <typename SysWrapperT>
SocketBase<SysWrapperT>::SocketBase(SocketBase &&aOther) noexcept
    : socketHandle_(std::exchange(aOther.socketHandle_, kInvalidSocket))
    , isOpen_(std::exchange(aOther.isOpen_, false))
    , isNonBlocking_(std::exchange(aOther.isNonBlocking_, false))
    , context_(aOther.context_)
    , handler_(nullptr)
{
    handler(aOther.handler_);
}

template <typename SysWrapperT>
SocketBase<SysWrapperT> &SocketBase<SysWrapperT>::operator=(
    SocketBase &&aOther) noexcept
{
    assert(!isOpen_ &&
           "Error: move assignment to opened socket is not allowed");
    std::swap(aOther.socketHandle_, socketHandle_);
    std::swap(aOther.isOpen_, isOpen_);
    std::swap(aOther.isNonBlocking_, isNonBlocking_);
    context_ = aOther.context_;
    handler_ = aOther.handler_;
    return *this;
}

template <typename SysWrapperT>
sock_t SocketBase<SysWrapperT>::nativeHandle() const noexcept
{
    return socketHandle_;
}

template <typename SysWrapperT>
bool SocketBase<SysWrapperT>::nonBlocking() const noexcept
{
    return isNonBlocking_;
}

template <typename SysWrapperT>
bool SocketBase<SysWrapperT>::isOpen() const noexcept
{
    return isOpen_;
}

template <typename SysWrapperT>
HandlerSelectBase<SysWrapperT> *SocketBase<SysWrapperT>::handler()
    const noexcept
{
    return handler_;
}

template <typename SysWrapperT>
void SocketBase<SysWrapperT>::handler(HandlerSelectBase<SysWrapperT> *aHandler)
{
    if (aHandler == handler_)
    {
        return;
    }
    handler_ = aHandler;
    if (!isOpen())
    {
        return;
    }
    if (aHandler != nullptr)
    {
        context_.get().executor().addSocket(this);
    }
    else
    {
        context_.get().executor().delSocket(this);
    }
}

template <typename SysWrapperT>
void SocketBase<SysWrapperT>::setIsSubscribed(bool aIsSubscribed)
{
    if (handler_)
    {
        if (aIsSubscribed)
        {
            context_.get().executor().addSocket(this);
        }
        else
        {
            context_.get().executor().delSocket(this);
        }
    }
}

template <typename SysWrapperT>
void SocketBase<SysWrapperT>::open(int socket_family, int socket_type,
                                   int protocol, std::error_code &aEc)
{
    const auto socketHandle =
        SysWrapperT::socket(socket_family, socket_type, protocol);
    if (kInvalidSocket == socketHandle)
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
        return;
    }
    socketHandle_ = socketHandle;
    isOpen_ = true;
    setIsSubscribed(true);
}

template <typename SysWrapperT>
void SocketBase<SysWrapperT>::bind(const uint8_t socket_family,
                                   const uint16_t aPort, std::error_code &aEc)
{
    Address sa(socket_family, aPort);
    const auto result =
        SysWrapperT::bind(socketHandle_, sa.nativeDataConst(),
                          static_cast<ndt::salen_t>(sa.capacity()));
    if (ndt::kSocketError == result)
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
    }
}

template <typename SysWrapperT>
std::size_t SocketBase<SysWrapperT>::sendTo(const Address &aDst, CBuffer aBuf)
{
    std::error_code ec;
    const auto bytesSent = SocketBase::sendTo(aDst, aBuf, ec);
    throw_if_error(ec);
    return static_cast<std::size_t>(bytesSent);
}

template <typename SysWrapperT>
std::size_t SocketBase<SysWrapperT>::sendTo(const Address &aDst, CBuffer aBuf,
                                            std::error_code &aEc)
{
    const auto bytesSent = SysWrapperT::sendto(
        socketHandle_, aBuf.data(), aBuf.size(), 0, aDst.nativeDataConst(),
        static_cast<ndt::salen_t>(aDst.capacity()));
    if (ndt::kSocketError == bytesSent)
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
    }
    return static_cast<std::size_t>(bytesSent);
}

template <typename SysWrapperT>
std::size_t SocketBase<SysWrapperT>::recvFrom(Buffer &aBuf, Address &aSender)
{
    std::error_code ec;
    const auto bytesReceived = SocketBase::recvFrom(aBuf, aSender, ec);
    throw_if_error(ec);
    return static_cast<std::size_t>(bytesReceived);
}

template <typename SysWrapperT>
std::size_t SocketBase<SysWrapperT>::recvFrom(Buffer &aBuf, Address &aSender,
                                              std::error_code &aEc)
{
    ndt::salen_t addrlen = static_cast<ndt::salen_t>(kV6Capacity);
    const auto bytesReceived =
        SysWrapperT::recvfrom(socketHandle_, aBuf.data(), aBuf.size(), 0,
                              aSender.nativeData(), &addrlen);
    if (ndt::kSocketError != bytesReceived)
    {
        aBuf.setSize(bytesReceived);
    }
    else
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
    }
    return static_cast<std::size_t>(bytesReceived);
}

template <typename SysWrapperT>
void SocketBase<SysWrapperT>::close()
{
    std::error_code ec;
    SocketBase::close(ec);
    throw_if_error(ec);
}

template <typename SysWrapperT>
void SocketBase<SysWrapperT>::close(std::error_code &aEc)
{
    if (!isOpen_)
    {
        return;
    }
    setIsSubscribed(false);
    const auto result = SysWrapperT::close(socketHandle_);
    if (ndt::kSocketError == result)
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
        return;
    }
    socketHandle_ = kInvalidSocket;
    isOpen_ = false;
}

template <typename SysWrapperT>
void SocketBase<SysWrapperT>::nonBlocking(const bool isNonBlocking)
{
    std::error_code ec;
    SocketBase::nonBlocking(isNonBlocking, ec);
    throw_if_error(ec);
}

template <typename SysWrapperT>
void SocketBase<SysWrapperT>::nonBlocking(const bool isNonBlocking,
                                          std::error_code &aEc) noexcept
{
#if _WIN32
    u_long isNonBlockingArg = isNonBlocking ? 1 : 0;
    const int flags =
        SysWrapperT::ioctlsocket(socketHandle_, FIONBIO, &isNonBlockingArg);
#else
    int flags = SysWrapperT::fcntl(socketHandle_, F_GETFL, 0);
    if (flags == kSocketError)
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
        return;
    }

    flags = isNonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    flags = SysWrapperT::fcntl(socketHandle_, F_SETFL, flags);
#endif
    if (flags == kSocketError)
    {
        aEc.assign(SysWrapperT::lastErrorCode(), std::system_category());
        return;
    }
    isNonBlocking_ = isNonBlocking;
}

template <typename FlagsT, typename SysWrapperT>
class Socket final : public SocketBase<SysWrapperT>
{
    friend class ExecutorSelect<SysWrapperT>;

   public:
    typedef FlagsT SocketT;
    typedef SysWrapperT SysCallsT;
    ~Socket();
    Socket() = delete;
    Socket(Socket &&) noexcept;
    Socket &operator=(Socket &&) noexcept;
    Socket(Context<SysWrapperT> &aContext, const FlagsT &flags) noexcept;
    Socket(Context<SysWrapperT> &aContext, const FlagsT &aFlags,
           uint16_t aPort);

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

template <typename FlagsT, typename SysWrapperT>
Socket<FlagsT, SysWrapperT>::~Socket()
{
}

template <typename FlagsT, typename SysWrapperT>
Socket<FlagsT, SysWrapperT>::Socket(Socket &&aOther) noexcept
    : SocketBase<SysWrapperT>(std::move(aOther))
    , flags_(std::move(aOther.flags_))
{
}

template <typename FlagsT, typename SysWrapperT>
Socket<FlagsT, SysWrapperT> &Socket<FlagsT, SysWrapperT>::operator=(
    Socket &&aOther) noexcept
{
    SocketBase<SysWrapperT>::operator=(std::move(aOther));
    std::swap(aOther.flags_, flags_);

    return *this;
}

template <typename FlagsT, typename SysWrapperT>
Socket<FlagsT, SysWrapperT>::Socket(Context<SysWrapperT> &aContext,
                                    const FlagsT &flags) noexcept
    : SocketBase<SysWrapperT>(aContext), flags_(flags)
{
}

template <typename FlagsT, typename SysWrapperT>
Socket<FlagsT, SysWrapperT>::Socket(Context<SysWrapperT> &aContext,
                                    const FlagsT &aFlags, uint16_t aPort)
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

template <typename FlagsT, typename SysWrapperT>
void Socket<FlagsT, SysWrapperT>::open()
{
    std::error_code ec;
    Socket<FlagsT, SysWrapperT>::open(ec);
    throw_if_error(ec);
}

template <typename FlagsT, typename SysWrapperT>
void Socket<FlagsT, SysWrapperT>::open(std::error_code &aEc)
{
    SocketBase<SysWrapperT>::open(flags_.sysFamily(), flags_.sysSocketType(),
                                  flags_.sysProtocol(), aEc);
}

template <typename FlagsT, typename SysWrapperT>
void Socket<FlagsT, SysWrapperT>::bind(const uint16_t aPort)
{
    std::error_code ec;
    Socket<FlagsT, SysWrapperT>::bind(aPort, ec);
    throw_if_error(ec);
}

template <typename FlagsT, typename SysWrapperT>
void Socket<FlagsT, SysWrapperT>::bind(const uint16_t aPort,
                                       std::error_code &aEc)
{
    SocketBase<SysWrapperT>::bind(flags_.sysFamily(), aPort, aEc);
}

template <typename FlagsT, typename SysWrapperT>
std::size_t Socket<FlagsT, SysWrapperT>::sendTo(const Address &aDst,
                                                CBuffer aBuf)
{
    return SocketBase<SysWrapperT>::sendTo(aDst, aBuf);
}

template <typename FlagsT, typename SysWrapperT>
std::size_t Socket<FlagsT, SysWrapperT>::sendTo(const Address &aDst,
                                                CBuffer aBuf,
                                                std::error_code &aEc)
{
    return SocketBase<SysWrapperT>::sendTo(aDst, aBuf, aEc);
}

template <typename FlagsT, typename SysWrapperT>
std::size_t Socket<FlagsT, SysWrapperT>::recvFrom(Buffer &aBuf,
                                                  Address &aSender)
{
    return SocketBase<SysWrapperT>::recvFrom(aBuf, aSender);
}

template <typename FlagsT, typename SysWrapperT>
std::size_t Socket<FlagsT, SysWrapperT>::recvFrom(Buffer &aBuf,
                                                  Address &aSender,
                                                  std::error_code &aEc)
{
    return SocketBase<SysWrapperT>::recvFrom(aBuf, aSender, aEc);
}

template <typename FlagsT, typename SysWrapperT>
void Socket<FlagsT, SysWrapperT>::close()
{
    SocketBase<SysWrapperT>::close();
}

template <typename FlagsT, typename SysWrapperT>
void Socket<FlagsT, SysWrapperT>::close(std::error_code &aEc)
{
    SocketBase<SysWrapperT>::close(aEc);
}

template <typename FlagsT, typename SysWrapperT>
bool Socket<FlagsT, SysWrapperT>::nonBlocking() const noexcept
{
    return SocketBase<SysWrapperT>::nonBlocking();
}

template <typename FlagsT, typename SysWrapperT>
void Socket<FlagsT, SysWrapperT>::nonBlocking(const bool isNonBlocking)
{
    SocketBase<SysWrapperT>::nonBlocking(isNonBlocking);
}

template <typename FlagsT, typename SysWrapperT>
void Socket<FlagsT, SysWrapperT>::nonBlocking(const bool isNonBlocking,
                                              std::error_code &aEc) noexcept
{
    SocketBase<SysWrapperT>::nonBlocking(isNonBlocking, aEc);
}

template <typename FlagsT, typename SysWrapperT>
FlagsT Socket<FlagsT, SysWrapperT>::flags() const noexcept
{
    return flags_;
}

}  // namespace ndt

#endif /* ndt_socket_h */