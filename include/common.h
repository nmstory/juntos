#pragma once

#ifdef _WIN32
#include "stdafx.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <winnt.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#endif