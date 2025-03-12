#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <arpa/inet.h>
#include <client.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

const char STUN_SERVER_IP[] = "127.0.0.1";
const int STUN_SERVER_PORT = 12345;
const short TIME_BETWEEN_HEARTBEATS = 5; // seconds

// Alert the STUN server of the existence of this client
int getOtherPeersDetails(int* sockfd, sockaddr_in* serverAddr) {
    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sockfd == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(STUN_SERVER_PORT);
    if (inet_pton(AF_INET, STUN_SERVER_IP, &(serverAddr->sin_addr)) <= 0) {
        std::cerr << "Invalid server IP address" << std::endl;
        close(*sockfd);
        return 1;
    }

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(0);  // Use any available port
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on all interfaces

    if (bind(*sockfd, (struct sockaddr*)&localAddr, sizeof(localAddr)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(*sockfd);
        return 1;
    }

    // Join the server
    std::string joinMessage = "JOIN";
    sendto(*sockfd, joinMessage.c_str(), joinMessage.length(), 0, (struct sockaddr*)serverAddr, sizeof(*serverAddr));
    
    char buffer[4096];
    socklen_t serverAddrLen = sizeof(*serverAddr);
    int bytesReceived = recvfrom(*sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)serverAddr, &serverAddrLen);
    
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Server response: " << buffer << std::endl;
    } 
    else {
        std::cerr << "No response from server or error occurred" << std::endl;
    }

    // Get the list of clients
    std::string listMessage = "LIST:";
    sendto(*sockfd, listMessage.c_str(), listMessage.length(), 0, (struct sockaddr*)serverAddr, sizeof(*serverAddr));
    bytesReceived = recvfrom(*sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)serverAddr, &serverAddrLen);
    if (bytesReceived > 0) {
        if (bytesReceived == 5) {
            std::puts("Currently waiting for other clients to connect! Hang on :)");
        }
        else {
            buffer[bytesReceived] = '\0';
            std::cout << "Client list: " << buffer << std::endl;
        }
    }

    return 0;
}

void sendHeartbeat(int sockfd, const sockaddr_in& serverAddr) {
    while (true) {
        std::string message = "HEARTBEAT:";
        sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        sleep(TIME_BETWEEN_HEARTBEATS);
    }
}

int main() {
    int* sockfd = new int(0);
    struct sockaddr_in* serverAddr = new struct sockaddr_in;

    int stunSuccess = getOtherPeersDetails(sockfd, serverAddr);
    if (stunSuccess != 0) return stunSuccess;

    std::thread heartbeatThread(sendHeartbeat, *sockfd, *serverAddr);
    heartbeatThread.detach();

    // @todo remove test code
    while(true){
        sleep(1);
    }

    close(*sockfd);
    return 0;
}