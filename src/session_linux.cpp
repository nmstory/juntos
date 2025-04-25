#ifdef __linux__

#include <session_linux.h>

LinuxSession::LinuxSession() {
	peers = new std::vector<Peer>();
	lastHeartbeatToStun = std::chrono::steady_clock::now();
}

LinuxSession::~LinuxSession() {
	close(sockFD);
}

bool LinuxSession::init(const int& portNumber) {
	char ip[20];
	strcpy(ip, "127.0.0.1");

	localAddr = populateAddress(ip, portNumber);
	sockFD = createSocket<int>(localAddr);

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

	// Set socket to non-blocking mode
	int flags = fcntl(sockFD, F_GETFL, 0);
	fcntl(sockFD, F_SETFL, flags | O_NONBLOCK);

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
	auto now = std::chrono::steady_clock::now();
	if ((now - lastHeartbeatToStun) > std::chrono::seconds(TIME_BETWEEN_HEARTBEATS)) {
		sendHeartbeatToStun<int>(sockFD, stunAddr);
		lastHeartbeatToStun = now;
	}

	auto [success, data, addr] = recvData<int>(sockFD);

	if (success) {
		std::string_view received_str(reinterpret_cast<const char*>(data.data()), data.size());
		std::cout << "Received data: " << received_str << std::endl;

		// Processing data received
		if (received_str == "PING") {
			std::string pongMessage = "PONG";
			sendto(sockFD, pongMessage.c_str(), pongMessage.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
			peers->push_back(Peer(addr));
		}
		else if (received_str == "PING") {
			peers->push_back(Peer(addr));
		}
	}

	return true;
}

#endif