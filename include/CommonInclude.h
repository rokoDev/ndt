#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "WinSock2.h"
#include "Windows.h"
#include "Ws2tcpip.h"
#pragma comment(lib, "Ws2_32.lib")

#else

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#endif