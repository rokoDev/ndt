#ifndef ndt_socket_h
#define ndt_socket_h

#include <cassert>

#include "address.h"
#include "common.h"
#include "exception.h"
#include "ndt/build_type_defs.h"
#include "nocopyable.h"
#include "utils.h"

namespace ndt
{
template <typename FlagsT, typename SFuncsT>
class Socket final : private Nocopyable
{
   public:
    ~Socket();
    Socket() = delete;
    Socket(Socket &&) noexcept;
    Socket &operator=(Socket &&) noexcept;
    explicit Socket(const FlagsT &flags) noexcept;
    Socket(const FlagsT &aFlags, uint16_t aPort);

    bool isOpen() const noexcept;
    void open();
    void open(std::error_code &aEc);
    void bind(const uint16_t aPort);
    void bind(const uint16_t aPort, std::error_code &aEc);
    std::size_t sendTo(const Address &aDst, ndt::cbuf_t aDataPtr,
                       const dlen_t aDataSize);
    std::size_t sendTo(const Address &aDst, ndt::cbuf_t aDataPtr,
                       const dlen_t aDataSize, std::error_code &aEc);
    std::size_t recvFrom(ndt::buf_t aDataPtr, ndt::dlen_t aDataSize,
                         Address &aSender);
    std::size_t recvFrom(ndt::buf_t aDataPtr, ndt::dlen_t aDataSize,
                         Address &aSender, std::error_code &aEc);
    void close();
    void close(std::error_code &aEc);

    const FlagsT &flags() const noexcept;

    bool nonBlocking() const noexcept;
    void nonBlocking(const bool isNonBlocking, std::error_code &aEc) noexcept;
    void nonBlocking(const bool isNonBlocking);

   protected:
    sock_t socketHandle_ = kInvalidSocket;
    FlagsT flags_;
    bool isOpen_ = false;
    bool isNonBlocking_ = false;
};

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::~Socket()
{
    assert(!isOpen_ && "Error: socket must be closed before destruction");
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(Socket &&aOther) noexcept
    : socketHandle_(std::exchange(aOther.socketHandle_, kInvalidSocket))
    , flags_(std::move(aOther.flags_))
    , isOpen_(std::exchange(aOther.isOpen_, false))
    , isNonBlocking_(std::exchange(aOther.isNonBlocking_, false))
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT> &Socket<FlagsT, SFuncsT>::operator=(
    Socket &&aOther) noexcept
{
    assert(!isOpen_ &&
           "Error: move assignment to opened socket is not allowed");
    std::swap(aOther.socketHandle_, socketHandle_);
    std::swap(aOther.flags_, flags_);
    std::swap(aOther.isOpen_, isOpen_);
    std::swap(aOther.isNonBlocking_, isNonBlocking_);

    return *this;
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(const FlagsT &flags) noexcept
    : socketHandle_(kInvalidSocket)
    , flags_(flags)
    , isOpen_(false)
    , isNonBlocking_(false)
{
}

template <typename FlagsT, typename SFuncsT>
Socket<FlagsT, SFuncsT>::Socket(const FlagsT &aFlags, uint16_t aPort)
    : Socket(aFlags)
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
bool Socket<FlagsT, SFuncsT>::isOpen() const noexcept
{
    return isOpen_;
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
    const auto socketHandle = SFuncsT::socket(
        flags_.sysFamily(), flags_.sysSocketType(), flags_.sysProtocol());
    if (kInvalidSocket == socketHandle)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
        return;
    }
    socketHandle_ = socketHandle;
    isOpen_ = true;
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
    Address sa(flags_.getFamily(), aPort);
    const auto result = SFuncsT::bind(socketHandle_, sa.nativeDataConst(),
                                      static_cast<ndt::salen_t>(sa.capacity()));
    if (ndt::kSocketError == result)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
    }
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::sendTo(const Address &aDst,
                                            ndt::cbuf_t aDataPtr,
                                            const ndt::dlen_t aLen)
{
    std::error_code ec;
    const auto bytesSent =
        Socket<FlagsT, SFuncsT>::sendTo(aDst, aDataPtr, aLen, ec);
    throw_if_error(ec);
    return static_cast<std::size_t>(bytesSent);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::sendTo(const Address &aDst,
                                            ndt::cbuf_t aDataPtr,
                                            const ndt::dlen_t aLen,
                                            std::error_code &aEc)
{
    const auto bytesSent = SFuncsT::sendto(
        socketHandle_, aDataPtr, aLen, 0, aDst.nativeDataConst(),
        static_cast<ndt::salen_t>(aDst.capacity()));
    if (ndt::kSocketError == bytesSent)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
    }
    return static_cast<std::size_t>(bytesSent);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::recvFrom(ndt::buf_t aDataPtr,
                                              ndt::dlen_t aDataSize,
                                              Address &aSender)
{
    std::error_code ec;
    const auto bytesReceived =
        Socket<FlagsT, SFuncsT>::recvFrom(aDataPtr, aDataSize, aSender, ec);
    throw_if_error(ec);
    return static_cast<std::size_t>(bytesReceived);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::recvFrom(ndt::buf_t aDataPtr,
                                              ndt::dlen_t aDataSize,
                                              Address &aSender,
                                              std::error_code &aEc)
{
    ndt::salen_t addrlen = static_cast<ndt::salen_t>(kV6Capacity);
    const auto bytesReceived = SFuncsT::recvfrom(
        socketHandle_, aDataPtr, aDataSize, 0, aSender.nativeData(), &addrlen);
    if (ndt::kSocketError == bytesReceived)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
    }
    return static_cast<std::size_t>(bytesReceived);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::close()
{
    std::error_code ec;
    Socket<FlagsT, SFuncsT>::close(ec);
    throw_if_error(ec);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::close(std::error_code &aEc)
{
    if (!isOpen_)
    {
        return;
    }
    const auto result = SFuncsT::close(socketHandle_);
    if (ndt::kSocketError == result)
    {
        aEc.assign(systemErrorCodeGetter(), std::system_category());
        return;
    }
    socketHandle_ = kInvalidSocket;
    isOpen_ = false;
}

template <typename FlagsT, typename SFuncsT>
const FlagsT &Socket<FlagsT, SFuncsT>::flags() const noexcept
{
    return flags_;
}

template <typename FlagsT, typename SFuncsT>
bool Socket<FlagsT, SFuncsT>::nonBlocking() const noexcept
{
    return isNonBlocking_;
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::nonBlocking(const bool isNonBlocking,
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
void Socket<FlagsT, SFuncsT>::nonBlocking(const bool isNonBlocking)
{
    std::error_code ec;
    Socket<FlagsT, SFuncsT>::nonBlocking(isNonBlocking, ec);
    throw_if_error(ec);
}

}  // namespace ndt

#endif /* ndt_socket_h */