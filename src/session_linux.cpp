#ifdef __linux__

#include <session_linux.h>

#include <iostream>
#include <string.h> //@todo needed?

LinuxSession::LinuxSession() {
	peers = new std::vector<Peer>();

}

LinuxSession::~LinuxSession() {
	close(sockFD);
}

bool LinuxSession::init(int portNumber) {
	char ip[20];
	strcpy(ip, "127.0.0.1");

	addr = populateAddress(ip, portNumber);
	sockFD = createSocket<int>(ip, addr);

	sockaddr_in stunAddr;
	stunAddr.sin_family = AF_INET;
	stunAddr.sin_port = htons(STUN_SERVER_PORT);
	if (inet_pton(AF_INET, STUN_SERVER_IP, &stunAddr.sin_addr) <= 0) {
		std::cerr << "Invalid server IP address" << std::endl;
		close(sockFD);
		return -1;
	}

	// Join the server
	std::string joinMessage = "JOIN";
	sendto(sockFD, joinMessage.c_str(), joinMessage.length(), 0, (struct sockaddr*) &stunAddr, sizeof(stunAddr));
	
	char buffer[4096];
	socklen_t serverAddrLen = sizeof(stunAddr);
	int bytesReceived = recvfrom(sockFD, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*) &stunAddr, &serverAddrLen);

	if (bytesReceived > 0) {
		buffer[bytesReceived] = '\0';
		std::cout << "Server response: " << buffer << std::endl;
	} 
	else {
		std::cerr << "No response from server or error occurred" << std::endl;
	}

	// Get the list of clients
	std::string listMessage = "LIST:";
	sendto(sockFD, listMessage.c_str(), listMessage.length(), 0, (struct sockaddr*)&stunAddr, sizeof(stunAddr));
	
	bytesReceived = recvfrom(sockFD, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&stunAddr, &serverAddrLen);
	if (bytesReceived > 0) {
		if (bytesReceived == 5) {
			std::puts("Currently waiting for other clients to connect! Hang on :)");
			return 0;
		}
		else {
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
		int bytesSent = sendto(sockFD, pingMessage.c_str(), pingMessage.length(), 0, (struct sockaddr*)&peer.sendAddr, sizeof(peer.sendAddr));
		if (bytesSent == -1) {
			std::cerr << "Error sending PING to peer " << peer.sendAddr.sin_addr.s_addr << ":" << ntohs(peer.sendAddr.sin_port) << std::endl;
		}
	}

	return true;
}

bool LinuxSession::update() {
	recvData<int>(sockFD);
	sendHeartbeat<int>(sockFD, addr);

	return true;
}

#endif