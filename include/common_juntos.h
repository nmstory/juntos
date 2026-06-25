#pragma once

// Includes for all platforms
#include <fcntl.h>

#ifdef _WIN32  // only windows
#  include <type_traits>

#  include <WinSock2.h>
#  include <winnt.h>
#  include <ws2tcpip.h>

#  include "stdafx.h"

using ssize_t = std::make_signed_t<size_t>;
#  include <winsock2.h>
using Socket = SOCKET;
#elif defined(__unix__) || defined(__APPLE__)
#  define JUNTOS_UNIX
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>

using Socket = int;
#endif
