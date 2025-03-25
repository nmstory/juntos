#pragma once

#include <common.h>

#include <array>
#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <string.h>
#include <tuple>

const short TIME_BETWEEN_HEARTBEATS = 5; // seconds

/*
	@brief Populate a socket address with an IP address and port
	@param ip: IP address to populate the socket address with
	@param port: Port to populate the socket address with
	@returns sockaddr_in: Populated sockaddr_in
*/
sockaddr_in populateAddress(const char* ip, const int& port);

/*
	@brief Output an error message
	@param error: Error message to output
*/
inline void outputError(const std::string& error) {
    std::cerr << error << std::endl;
#if _WIN32
    std::cout << WSAGetLastError() << std::endl;
#elif __linux__
    std::perror("error");
#endif
}

/*
	@brief Creates a socket
	@param addr: Address to bind the socket to
	@returns: created socket
*/
template <typename T>
T createSocket(sockaddr_in& addr) {
	T s;
	
#ifdef _WIN32
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET){
#elif __linux__
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
#endif
		std::puts("Error creating socket!\n");
#ifdef _WIN32
		std::cout << "Error initialising socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		closesocket(s);
#elif __linux__
		close(s);
#endif
		return 0;
	}
	
	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1){
		outputError("Error binding socket");
		return -1;
	}
	
	return s;
}

/*
	@brief Sends data to an address via a socket
	@param sendSockFD: Socket to send data from
	@param recvAddr: Address to send data to
	@param string: Data to send
	@param length: Length of data to send
	@returns bool: whether data was sent successfully
*/
template <typename T>
bool sendData(const T& sendSockFD, const sockaddr_in& recvAddr, const std::span<const std::byte>& string, const size_t& length) {
	int byteCount = sendto(sendSockFD, string, length, 0, (struct sockaddr *)&recvAddr, sizeof(recvAddr));

	if (byteCount > 0) {
		return true;
	}
	else {
		outputError("Error sending data");
	}
	
	return false;
}

/*
	@brief Receive data from a socket
	@param recvSockFD: Socket to receive data from
	@returns std::tuple<bool, std::vector<char>, sockaddr_in>
		- success_flag: whether data was received successfully
		- received_data: the data received from the socket
		- sender_address: address of the sender
*/ 
template <typename T>
std::tuple<bool, std::span<const std::byte>, sockaddr_in> recvData(T recvSockFD) {
	std::array<std::byte, 65507> buffer;  // Max safe UDP payload size
    sockaddr_in sender_addr{};
    socklen_t addr_len = sizeof(sender_addr);

    ssize_t byteCount = recvfrom(
        recvSockFD,
        reinterpret_cast<char*>(buffer.data()), //reinterpret_cast<char*> as Winsock uses char*, whilst Linux uses void*
        static_cast<int>(buffer.size()),
        0,
        reinterpret_cast<sockaddr*>(&sender_addr),
        &addr_len
    );

    if (byteCount > 0) {
        return {true, std::span<const std::byte>(buffer.data(), byteCount), sender_addr};
    }
    
#ifdef _WIN32
	if (byteCount == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            std::cerr << "recvfrom error: " << err << std::endl;
        }
    }
#elif __linux__
    if (byteCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        std::perror("recvfrom error");
    }
#endif
    
    return {false, {}, {}};
}

/*
	@brief Send a heartbeat to the STUN server
	@param sockfd: Socket to send the heartbeat from
	@param serverAddr: Address of the STUN server	
*/
template <typename T>
void sendHeartbeatToStun(const T& sockfd, const sockaddr_in& serverAddr) {
	std::string message = "HEARTBEAT:";
	sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
}