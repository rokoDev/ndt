#include "ndt/system_wrappers.h"

namespace ndt
{
int System::bind(sock_t sockfd, const struct sockaddr *addr,
                 ndt::salen_t addrlen)
{
    return ::bind(sockfd, addr, addrlen);
}

sdlen_t System::recvfrom(sock_t sockfd, ndt::buf_t buf, ndt::dlen_t len,
                         int flags, struct sockaddr *src_addr,
                         ndt::salen_t *addrlen)
{
    return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

sdlen_t System::sendto(sock_t sockfd, ndt::cbuf_t buf, ndt::dlen_t len,
                       int flags, const struct sockaddr *dest_addr,
                       ndt::salen_t addrlen)
{
    return ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

sock_t System::socket(int socket_family, int socket_type, int protocol)
{
    return ::socket(socket_family, socket_type, protocol);
}

int System::close(sock_t fd) { return ndt::socketcloser(fd); }

const char *System::inet_ntop(int af, const void *src, char *dst, salen_t size)
{
    return ::inet_ntop(af, src, dst, size);
}

int System::inet_pton(int af, const char *src, void *dst)
{
    return ::inet_pton(af, src, dst);
}
}  // namespace ndt