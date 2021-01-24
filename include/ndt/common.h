/** @file common.h
    @brief Header with typedefs to make type designation platform independent.
 */
#ifndef ndt_common_h
#define ndt_common_h

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#if defined(_MSC_VER)
#include <BaseTsd.h>
using ssize_t = SSIZE_T;
#endif

#include "WinSock2.h"
#include "Windows.h"
#include "Ws2tcpip.h"
#pragma comment(lib, "Ws2_32.lib")
#include <cstddef>
namespace ndt
{
/*! \typedef using buf_t = char*;
    \brief buf_t - instances of this type contain pointer to byte array.
 */
using buf_t = char*;

/*! \typedef using salen_t = int;
    \brief salen_t  -  instances of this type hold length of data pointed by
   pointer to sockaddr. In expression with variable p where p declared as:
   sockaddr * p; p can actually point to either: sockaddr_in or sockaddr_in6.
   Hence to clarify how many bytes are pointed by p we can use an instance of
   salen_t type.
 */
using salen_t = int;

/*! \typedef using dlen_t = int;
    \brief dlen_t - instances of this type hold length of data pointed by
   variable of buf_t or cbuf_t types.
 */
using dlen_t = int;

/*! \typedef using sdlen_t = int;
    \brief sdlen_t - instances of this type hold values returned from functions
   like recvfrom or sendto.
 */
using sdlen_t = int;
}  // namespace ndt

#else

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace ndt
{
using buf_t = void*;
using salen_t = socklen_t;
using dlen_t = size_t;
using sdlen_t = ssize_t;
}  // namespace ndt

#endif

/*! \namespace ndt
    \brief ndt - namespace which contains all stuff provided by this library.
 */
namespace ndt
{
/*! \typedef using cbuf_t = const buf_t;
    \brief cbuf_t - instances of this type hold pointer to immutabe byte array.
 */
using cbuf_t = const buf_t;

/*! \typedef using sock_t = decltype(::socket(0, 0, 0));
    \brief sock_t - instance of this type contains socket descriptor returned by
   socket function.
 */
using sock_t = decltype(::socket(0, 0, 0));

#ifdef _WIN32
inline constexpr sock_t kInvalidSocket = INVALID_SOCKET;
inline constexpr int kSocketError = SOCKET_ERROR;
inline const auto socketcloser = ::closesocket;
#define err_code WSAGetLastError()
#define FUNC_INFO __FUNCSIG__
using ch_t = TCHAR;
#else
inline constexpr sock_t kInvalidSocket = -1;
inline constexpr int kSocketError = -1;
inline constexpr auto socketcloser = ::close;
#define err_code errno
#define FUNC_INFO __PRETTY_FUNCTION__
using ch_t = char;
#endif

std::size_t getSysErrorDescr(const int aErrorCode, ch_t *aBuf,
                             const std::size_t aBufLen);
}  // namespace ndt

#endif /* ndt_common_h */
