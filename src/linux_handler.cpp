#pragma once
#ifdef __linux__

#include <linux_handler.h>

#include <iostream>
#include <string.h>

sockaddr_in populateAddress(const char* ip, const int port){
    struct sockaddr_in si;

    memset(&si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(port);

    // Convert IP address to binary
    if (inet_pton(AF_INET, ip, &si.sin_addr) <= 0) {
        std::cout << "Error converting IP address to binary, for IP " << ip << " and port " << port << std::endl;
        perror("inet_pton failed");
    }

    return si;
}

int createSocket(const char* ip, sockaddr_in addr) {
    int s;
    
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		std::puts("Error creating socket!\n");
        close(s);
		return 0;
	}

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1){
		std::puts("Error binding to socket!\n");
        perror("Bind failed");
		return -1;
	}

    return s;
}


bool sendData(int sendSockFD, sockaddr_in* recvAddr, const char* string, const size_t length){
    int byteCount = sendto(sendSockFD, string, length, 0, (struct sockaddr *)&recvAddr, sizeof(recvAddr)); //0 flags

    if (byteCount > 0) {
        std::cout << "message sent successfully" << std::endl;
        // @todo would want to close socket etc., as we normally do
        return true;
    }
    else {
        perror("Error sending!\n");
        // @todo would want to close socket etc., as we normally do
    }
    
    return false;
}   

bool recvData(int recvSockFD){
    char buffer[200];
    int byteCount = recv(recvSockFD, buffer, 200, 0);
    
    if (byteCount > 0) {
        std::cout << "Message received: " << buffer << std::endl;
        // @todo would want to close socket etc., as we normally do
        return true;
    }

    perror("Error receving!\n");
    return false;
    
}
#endif