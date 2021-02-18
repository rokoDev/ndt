#ifndef ndt_socket_h
#define ndt_socket_h

#include "address.h"
#include "common.h"
#include "exception.h"
#include "utils.h"

namespace ndt
{
namespace exception
{
inline const std::string kSocketAlreadyOpened = "socket already opened";
inline const std::string kSocketMustBeOpenToBind =
    "socket must be opened to bind";
inline const std::string kSocketOpen = "socket error";
inline const std::string kSocketBind = "bind error";
inline const std::string kSocketSendTo = "sendto error";
inline const std::string kSocketRecvFrom = "recvfrom error";
inline const std::string kSocketClose = "close error";
}  // namespace exception

enum class eSocketErrorCode
{
    kSuccess = 0,
    kAlreadyOpened,
    kMustBeOpenToBind,
    kOpen,
    kBind,
    kSendTo,
    kRecvFrom,
    kClose
};
}  // namespace ndt

namespace std
{
// Tell the C++ 11 STL metaprogramming that enum ndt::eSocketErrorCode
// is registered with the standard error code system
template <>
struct is_error_code_enum<ndt::eSocketErrorCode> : true_type
{
};
}  // namespace std

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
    std::error_code ec;
    Socket<FlagsT, SFuncsT>::open(ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::open(std::error_code &aEc)
{
    if (isOpen())
    {
        aEc = eSocketErrorCode::kAlreadyOpened;
        return;
    }
    const auto socketHandle = SFuncsT::socket(
        _flags.sysFamily(), _flags.sysSocketType(), _flags.sysProtocol());
    if (kInvalidSocket == socketHandle)
    {
        aEc = eSocketErrorCode::kOpen;
        return;
    }
    _socketHandle = socketHandle;
    _isOpen = true;
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::bind(const uint16_t aPort)
{
    std::error_code ec;
    Socket<FlagsT, SFuncsT>::bind(aPort, ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::bind(const uint16_t aPort, std::error_code &aEc)
{
    if (!isOpen())
    {
        aEc = eSocketErrorCode::kMustBeOpenToBind;
        return;
    }
    Address sa(_flags.getFamily(), aPort);
    const auto result = SFuncsT::bind(_socketHandle, sa.nativeDataConst(),
                                      static_cast<ndt::salen_t>(sa.capacity()));
    if (ndt::kSocketError == result)
    {
        aEc = eSocketErrorCode::kBind;
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
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
    return static_cast<std::size_t>(bytesSent);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::sendTo(const Address &aDst,
                                            ndt::cbuf_t aDataPtr,
                                            const ndt::dlen_t aLen,
                                            std::error_code &aEc)
{
    const auto bytesSent = SFuncsT::sendto(
        _socketHandle, aDataPtr, aLen, 0, aDst.nativeDataConst(),
        static_cast<ndt::salen_t>(aDst.capacity()));
    if (ndt::kSocketError == bytesSent)
    {
        aEc = eSocketErrorCode::kSendTo;
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
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
    return static_cast<std::size_t>(bytesReceived);
}

template <typename FlagsT, typename SFuncsT>
std::size_t Socket<FlagsT, SFuncsT>::recvFrom(ndt::buf_t aDataPtr,
                                              ndt::dlen_t aDataSize,
                                              Address &aSender,
                                              std::error_code &aEc)
{
    ndt::salen_t addrlen = static_cast<ndt::salen_t>(aSender.capacityV6());
    const auto bytesReceived = SFuncsT::recvfrom(
        _socketHandle, aDataPtr, aDataSize, 0, aSender.nativeData(), &addrlen);
    if (ndt::kSocketError == bytesReceived)
    {
        aEc = eSocketErrorCode::kRecvFrom;
    }
    return static_cast<std::size_t>(bytesReceived);
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::close()
{
    std::error_code ec;
    Socket<FlagsT, SFuncsT>::close(ec);
    if (ec)
    {
        ndt::Error ndtError(ec);
        throw ndtError;
    }
}

template <typename FlagsT, typename SFuncsT>
void Socket<FlagsT, SFuncsT>::close(std::error_code &aEc)
{
    if (!_isOpen)
    {
        return;
    }
    const auto result = SFuncsT::close(_socketHandle);
    if (ndt::kSocketError == result)
    {
        aEc = eSocketErrorCode::kClose;
    }
    _socketHandle = kInvalidSocket;
    _isOpen = false;
}

template <typename FlagsT, typename SFuncsT>
const FlagsT &Socket<FlagsT, SFuncsT>::flags() const noexcept
{
    return _flags;
}

class SocketErrorCategory : public std::error_category
{
   public:
    virtual const char *name() const noexcept override final;
    virtual std::string message(int c) const override final;
    virtual std::error_condition default_error_condition(
        int c) const noexcept override final;
};

inline std::error_code make_error_code(eSocketErrorCode e)
{
    static SocketErrorCategory c;
    return {static_cast<int>(e), c};
}

}  // namespace ndt

#endif /* ndt_socket_h */