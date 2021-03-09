#ifndef ndt_system_wrappers_h
#define ndt_system_wrappers_h

#include "common.h"

namespace ndt
{
class System final
{
    ~System() = delete;
    System() = delete;
    System(const System &) = delete;
    System &operator=(const System &) = delete;
    System(System &&) = delete;
    System &operator=(System &&) = delete;

   public:
    static int bind(sock_t sockfd, const struct sockaddr *addr,
                    ndt::salen_t addrlen) noexcept;
    static sdlen_t recvfrom(sock_t sockfd, ndt::buf_t buf, ndt::dlen_t len,
                            int flags, struct sockaddr *src_addr,
                            ndt::salen_t *addrlen) noexcept;
    static sdlen_t sendto(sock_t sockfd, ndt::cbuf_t buf, ndt::dlen_t len,
                          int flags, const struct sockaddr *dest_addr,
                          ndt::salen_t addrlen) noexcept;
    static sock_t socket(int socket_family, int socket_type,
                         int protocol) noexcept;
    static int close(sock_t fd) noexcept;
    static const char *inet_ntop(int af, const void *src, char *dst,
                                 salen_t size) noexcept;
    static int inet_pton(int af, const char *src, void *dst) noexcept;
#if _WIN32
    static int ioctlsocket(sock_t s, long cmd, u_long *argp) noexcept;
#else
    static int fcntl(sock_t s, int cmd, int arg) noexcept;
#endif
};
}  // namespace ndt

#endif /* ndt_system_wrappers_h */