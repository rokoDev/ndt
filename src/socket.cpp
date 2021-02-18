#include "ndt/socket.h"

namespace ndt
{
namespace details
{
int SocketFuncs::bind(sock_t sockfd, const struct sockaddr* addr,
                      ndt::salen_t addrlen)
{
    return ::bind(sockfd, addr, addrlen);
}

sdlen_t SocketFuncs::recvfrom(sock_t sockfd, ndt::buf_t buf, ndt::dlen_t len,
                              int flags, struct sockaddr* src_addr,
                              ndt::salen_t* addrlen)
{
    return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

sdlen_t SocketFuncs::sendto(sock_t sockfd, ndt::cbuf_t buf, ndt::dlen_t len,
                            int flags, const struct sockaddr* dest_addr,
                            ndt::salen_t addrlen)
{
    return ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

sock_t SocketFuncs::socket(int socket_family, int socket_type, int protocol)
{
    return ::socket(socket_family, socket_type, protocol);
}

int SocketFuncs::close(sock_t fd) { return ndt::socketcloser(fd); }

}  // namespace details

const char* SocketErrorCategory::name() const noexcept
{
    return "eSocketErrorCode";
}

std::string SocketErrorCategory::message(int c) const
{
    switch (static_cast<eSocketErrorCode>(c))
    {
        case eSocketErrorCode::kSuccess:
            return "success";
        case eSocketErrorCode::kAlreadyOpened:
            return exception::kSocketAlreadyOpened;
        case eSocketErrorCode::kMustBeOpenToBind:
            return exception::kSocketMustBeOpenToBind;
        case eSocketErrorCode::kOpen:
            return exception::kSocketOpen;
        case eSocketErrorCode::kBind:
            return exception::kSocketBind;
        case eSocketErrorCode::kSendTo:
            return exception::kSocketSendTo;
        case eSocketErrorCode::kRecvFrom:
            return exception::kSocketRecvFrom;
        case eSocketErrorCode::kClose:
            return exception::kSocketClose;
        default:
            return "unknown";
    }
}

std::error_condition SocketErrorCategory::default_error_condition(
    int c) const noexcept
{
    switch (static_cast<eSocketErrorCode>(c))
    {
        case eSocketErrorCode::kAlreadyOpened:
            return make_error_condition(std::errc::operation_not_supported);
        case eSocketErrorCode::kMustBeOpenToBind:
            return make_error_condition(std::errc::not_a_socket);
        case eSocketErrorCode::kOpen:
        case eSocketErrorCode::kBind:
        case eSocketErrorCode::kSendTo:
        case eSocketErrorCode::kRecvFrom:
        case eSocketErrorCode::kClose:
            return make_error_condition(std::errc::invalid_argument);
        default:
            // I have no mapping for this code
            return std::error_condition(c, *this);
    }
}

}  // namespace ndt