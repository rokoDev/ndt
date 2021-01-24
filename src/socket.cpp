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
}  // namespace ndt
