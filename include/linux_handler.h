#pragma once

#ifdef __linux__

#include <common.h>

sockaddr_in populateAddress(const char* ip, const int port);
int createSocket(const char* ip, sockaddr_in addr);
bool sendData(int sendSockFD, sockaddr_in* recvAddr, const char* string, const size_t length);
bool recvData(int recvSockFD);

#endif