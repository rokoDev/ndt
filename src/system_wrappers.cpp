#include "ndt/system_wrappers.h"

namespace ndt
{
int System::bind(sock_t sockfd, const struct sockaddr *addr,
                 ndt::salen_t addrlen) noexcept
{
    return ::bind(sockfd, addr, addrlen);
}

sdlen_t System::recvfrom(sock_t sockfd, ndt::bufp_t buf, ndt::dlen_t len,
                         int flags, struct sockaddr *src_addr,
                         ndt::salen_t *addrlen) noexcept
{
    return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

sdlen_t System::sendto(sock_t sockfd, ndt::cbufp_t buf, ndt::dlen_t len,
                       int flags, const struct sockaddr *dest_addr,
                       ndt::salen_t addrlen) noexcept
{
    return ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

sock_t System::socket(int socket_family, int socket_type, int protocol) noexcept
{
    return ::socket(socket_family, socket_type, protocol);
}

int System::close(sock_t fd) noexcept { return ndt::socketcloser(fd); }

const char *System::inet_ntop(int af, const void *src, char *dst,
                              salen_t size) noexcept
{
    return ::inet_ntop(af, src, dst, size);
}

int System::inet_pton(int af, const char *src, void *dst) noexcept
{
    return ::inet_pton(af, src, dst);
}

int System::select(int nfds, fd_set *readfds, fd_set *writefds,
                   fd_set *exceptfds, struct timeval *timeout) noexcept
{
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}
#if _WIN32
int System::ioctlsocket(sock_t s, long cmd, u_long *argp) noexcept
{
    return ::ioctlsocket(s, cmd, argp);
}
#else
int System::fcntl(sock_t s, int cmd, int arg) noexcept
{
    return ::fcntl(s, cmd, arg);
}
#endif
}  // namespace ndt