#include "Socket.h"

namespace net
{
namespace details
{
int SocketFuncs::bind(int sockfd, const struct sockaddr* addr,
                      socklen_t addrlen)
{
    return ::bind(sockfd, addr, addrlen);
}

ssize_t SocketFuncs::recvfrom(int sockfd, void* buf, size_t len, int flags,
                              struct sockaddr* src_addr, socklen_t* addrlen)
{
    return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

ssize_t SocketFuncs::sendto(int sockfd, const void* buf, size_t len, int flags,
                            const struct sockaddr* dest_addr, socklen_t addrlen)
{
    return ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

int SocketFuncs::socket(int socket_family, int socket_type, int protocol)
{
    return ::socket(socket_family, socket_type, protocol);
}

int SocketFuncs::close(int fd) { return fd; }

}  // namespace details
}  // namespace net