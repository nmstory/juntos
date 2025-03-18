#ifdef _WIN32

#include <session_windows.h>

#include <string.h> //@todo needed?
#include <iostream>

WindowsSession::WindowsSession() {
	peers = new std::vector<Peer>();

	int wsaerr;
	WORD wVersionRequested = MAKEWORD(2, 2);
	wsaerr = WSAStartup(wVersionRequested, &wsaData);
	if (wsaerr != 0) {
		std::cout << "Failed to locate the Winsock dll" << std::endl;
	}
}

WindowsSession::~WindowsSession() {
	closesocket(socket);
	WSACleanup();
}

bool WindowsSession::init(int portNumber) {
	char ip[20];
	strcpy(ip, "127.0.0.1");

	addr = populateAddress(ip, portNumber);
	socket = createSocket<SOCKET>(ip, addr);

	sockaddr_in stunAddr;
	stunAddr.sin_family = AF_INET;
	stunAddr.sin_port = htons(STUN_SERVER_PORT);
	if (inet_pton(AF_INET, STUN_SERVER_IP, &stunAddr.sin_addr) <= 0) {
		std::cerr << "Invalid server IP address" << std::endl;
		closesocket(socket);
		return -1;
	}

	// Join the server
	std::string joinMessage = "JOIN";
	sendto(socket, joinMessage.c_str(), joinMessage.length(), 0, (struct sockaddr*) &stunAddr, sizeof(stunAddr));
	
	char buffer[4096];
	socklen_t serverAddrLen = sizeof(stunAddr);
	int bytesReceived = recvfrom(socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*) &stunAddr, &serverAddrLen);

	if (bytesReceived > 0) {
		buffer[bytesReceived] = '\0';
		std::cout << "Server response: " << buffer << std::endl;
	}
	else {
		std::cerr << "No response from server or error occurred. recvfrom returned: " << bytesReceived << std::endl;
		int err = WSAGetLastError(); 
		std::cerr << "recvfrom error: " << err << std::endl;
	}

	// Get the list of clients
	std::string listMessage = "LIST:";
	sendto(socket, listMessage.c_str(), listMessage.length(), 0, (struct sockaddr*)&stunAddr, sizeof(stunAddr));
	
	bytesReceived = recvfrom(socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&stunAddr, &serverAddrLen);
	if (bytesReceived > 0) {
		if (bytesReceived == 5) {
			std::puts("Currently waiting for other clients to connect! Hang on :)");
			return 0;
		}
		else {
			buffer[bytesReceived] = '\0';
			
			char* ip;
			char* splitterIndex = strtok(buffer, ":;");

			while (splitterIndex != NULL) {
				ip = splitterIndex;
				splitterIndex = strtok(NULL, ":;");
				peers->push_back(Peer(populateAddress(ip, atoi(splitterIndex))));
				splitterIndex = strtok(NULL, ":;");
			}
		}
	}

	// ping each client
	for (Peer peer : *peers) {
		std::string pingMessage = "PING";
		int bytesSent = sendto(socket, pingMessage.c_str(), pingMessage.length(), 0, (struct sockaddr*)&peer.sendAddr, sizeof(peer.sendAddr));
		if (bytesSent == -1) {
			std::cerr << "Error sending PING to peer " << peer.sendAddr.sin_addr.s_addr << ":" << ntohs(peer.sendAddr.sin_port) << std::endl;
		}
	}

	return true;
}

bool WindowsSession::update() {
	recvData<SOCKET>(socket);
	sendHeartbeat<SOCKET>(socket, addr);

	return true;
}

#endif