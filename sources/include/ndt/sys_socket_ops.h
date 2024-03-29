#ifndef ndt_sys_socket_ops_h
#define ndt_sys_socket_ops_h

#include "common.h"
#include "sys_error_code.h"
#include "useful_base_types.h"

namespace ndt
{
class SysSocketOps
{
   public:
    static int bind(sock_t sockfd, const struct sockaddr *addr,
                    ndt::salen_t addrlen) noexcept;
    static sdlen_t recvfrom(sock_t sockfd, ndt::bufp_t buf, ndt::dlen_t len,
                            int flags, struct sockaddr *src_addr,
                            ndt::salen_t *addrlen) noexcept;
    static sdlen_t sendto(sock_t sockfd, ndt::cbufp_t buf, ndt::dlen_t len,
                          int flags, const struct sockaddr *dest_addr,
                          ndt::salen_t addrlen) noexcept;
    static sock_t socket(int socket_family, int socket_type,
                         int protocol) noexcept;
    static int close(sock_t fd) noexcept;
    static const char *inet_ntop(int af, const void *src, char *dst,
                                 salen_t size) noexcept;
    static int inet_pton(int af, const char *src, void *dst) noexcept;
    [[nodiscard]] static int select(int nfds, fd_set *readfds, fd_set *writefds,
                                    fd_set *exceptfds,
                                    struct timeval *timeout) noexcept;
#if _WIN32
    static int ioctlsocket(sock_t s, long cmd, u_long *argp) noexcept;
    static int WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData) noexcept;
    static int WSACleanup() noexcept;
#else
    static int fcntl(sock_t s, int cmd, int arg) noexcept;
#endif
};

class SocketOps
    : public NoConstructibleNoDestructible
    , public SysSocketOps
    , public SysErrorCode
{
};
}  // namespace ndt

#endif /* ndt_sys_socket_ops_h */
