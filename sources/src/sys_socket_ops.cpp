#include "ndt/sys_socket_ops.h"

namespace ndt
{
int SysSocketOps::bind(sock_t sockfd, const struct sockaddr *addr,
                       ndt::salen_t addrlen) noexcept
{
    return ::bind(sockfd, addr, addrlen);
}

sdlen_t SysSocketOps::recvfrom(sock_t sockfd, ndt::bufp_t buf, ndt::dlen_t len,
                               int flags, struct sockaddr *src_addr,
                               ndt::salen_t *addrlen) noexcept
{
    return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

sdlen_t SysSocketOps::sendto(sock_t sockfd, ndt::cbufp_t buf, ndt::dlen_t len,
                             int flags, const struct sockaddr *dest_addr,
                             ndt::salen_t addrlen) noexcept
{
    return ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

sock_t SysSocketOps::socket(int socket_family, int socket_type,
                            int protocol) noexcept
{
    return ::socket(socket_family, socket_type, protocol);
}

int SysSocketOps::close(sock_t fd) noexcept { return ndt::socketcloser(fd); }

const char *SysSocketOps::inet_ntop(int af, const void *src, char *dst,
                                    salen_t size) noexcept
{
    return ::inet_ntop(af, src, dst, size);
}

int SysSocketOps::inet_pton(int af, const char *src, void *dst) noexcept
{
    return ::inet_pton(af, src, dst);
}

int SysSocketOps::select(int nfds, fd_set *readfds, fd_set *writefds,
                         fd_set *exceptfds, struct timeval *timeout) noexcept
{
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}
#if _WIN32
int SysSocketOps::ioctlsocket(sock_t s, long cmd, u_long *argp) noexcept
{
    return ::ioctlsocket(s, cmd, argp);
}

int SysSocketOps::WSAStartup(WORD wVersionRequired,
                             LPWSADATA lpWSAData) noexcept
{
    return ::WSAStartup(wVersionRequired, lpWSAData);
    ;
}

int SysSocketOps::WSACleanup() noexcept { return ::WSACleanup(); }

#else
int SysSocketOps::fcntl(sock_t s, int cmd, int arg) noexcept
{
    return ::fcntl(s, cmd, arg);
}
#endif
}  // namespace ndt