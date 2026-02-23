#pragma once

// Includes for all platforms
#include <fcntl.h>

#ifdef _WIN32 // only windows
#include "stdafx.h"
#include <type_traits>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <winnt.h>

using ssize_t = std::make_signed_t<size_t>;
#include <winsock2.h>
using Socket = SOCKET;
#elif defined(__unix__) || defined(__APPLE__)
#define JUNTOS_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using Socket = int;
#endif
