#pragma once

#include <common.h>

#include <iostream>
#include <string.h>
#include <thread>

const short TIME_BETWEEN_HEARTBEATS = 5; // seconds

sockaddr_in populateAddress(const char* ip, const int port);

template <typename T>
T createSocket(const char* ip, sockaddr_in addr) {
	T s;
	
#ifdef _WIN32
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET){
#elif __linux__
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
#endif
		std::puts("Error creating socket!\n");
#ifdef _WIN32
		std::cout << "Error initialising socket: " << WSAGetLastError() << std::endl
		WSACleanup();
		closesocket(s);
#elif __linux__
		close(s);
#endif
		return 0;
	}

	
	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1){
		std::puts("Error binding to socket!\n");
		perror("Bind failed");
		return -1;
	}
	
	return s;
}

template <typename T>
bool sendData(T sendSockFD, sockaddr_in* recvAddr, const char* string, const size_t length){
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

template <typename T>
bool recvData(T recvSockFD){
	char buffer[200];
	int byteCount = recv(recvSockFD, buffer, 200, 0);
	
	if (byteCount > 0) {
		std::cout << "Message received: " << buffer << std::endl;
		// @todo would want to close socket etc., as we normally do
		return true;
	}

	perror("Error receiving!\n");
	return false;
	
}

template <typename T>
void sendHeartbeat(T sockfd, const sockaddr_in& serverAddr) {
	while (true) {
		std::string message = "HEARTBEAT:";
		sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		std::this_thread::sleep_for(std::chrono::seconds(TIME_BETWEEN_HEARTBEATS));
	}
}