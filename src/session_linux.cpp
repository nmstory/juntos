#include <common_juntos.h>

#ifdef JUNTOS_UNIX

#include <session_linux.h>
#include <cstring>

LinuxSession::LinuxSession() {
	lastHeartbeatToStun = std::chrono::steady_clock::now();
}

LinuxSession::~LinuxSession() {
	close(sockFD);
}

bool LinuxSession::initSessionToStun(const int& portNumber) {
	// TODO: if already initialised, inform and back out
	stunEnabled = true;
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
	static constexpr char joinMessage[] = "JOIN";
	sendto(sockFD, joinMessage, sizeof(joinMessage) - 1, 0, (struct sockaddr*) &stunAddr, sizeof(stunAddr));
	
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
	static constexpr char listMessage[] = "LIST:";
	sendto(sockFD, listMessage, sizeof(listMessage) - 1, 0, (struct sockaddr*)&stunAddr, sizeof(stunAddr));
	
	bytesReceived = recvfrom(sockFD, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&stunAddr, &serverAddrLen);
	if (bytesReceived > 0) {
		buffer[bytesReceived] = '\0';
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
				if (splitterIndex == NULL) break;
				addPeerIfNew(populateAddress(ip, atoi(splitterIndex)));
				splitterIndex = strtok(NULL, ":;");
			}
		}
	}

	// Set socket to non-blocking mode
	int flags = fcntl(sockFD, F_GETFL, 0);
	fcntl(sockFD, F_SETFL, flags | O_NONBLOCK);

	// ping each client
	static constexpr char pingMessage[] = "PING";
	for (const Peer& peer : peers) {
		int bytesSent = sendto(sockFD, pingMessage, sizeof(pingMessage) - 1, 0, (struct sockaddr*)&peer.sendAddr, sizeof(peer.sendAddr));
		if (bytesSent == -1) {
			std::cerr << "Error sending PING to peer " << peer.sendAddr.sin_addr.s_addr << ":" << ntohs(peer.sendAddr.sin_port) << std::endl;
		}
	}

	return true;
}

bool LinuxSession::initSessionSolo(const std::string& localHostname, const int& portNumber, std::optional<std::chrono::milliseconds> recvTimeout) {
	// TODO: if already initialised, inform and back out
	localAddr = populateAddress(localHostname.c_str(), portNumber);
	sockFD = createSocket<int>(localAddr);

	if (recvTimeout) {
        struct timeval tv{};
        tv.tv_sec  = recvTimeout->count() / 1000;
        tv.tv_usec = (recvTimeout->count() % 1000) * 1000;
        if (setsockopt(sockFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            std::cerr << "Failed to set SO_RCVTIMEO: " << std::strerror(errno) << std::endl;
        }
    }

	return true;
}

Peer LinuxSession::setupPeer(const std::string& destHostname, const int& destPort) {
	sockaddr_in peerAddr = populateAddress(destHostname.c_str(), destPort);
	addPeerIfNew(peerAddr);

	// Bootstrap the handshake so the remote end discovers us via its update() PING handler
	static constexpr char pingMessage[] = "PING";
	sendto(sockFD, pingMessage, sizeof(pingMessage) - 1, 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));

	return Peer(peerAddr);
}

std::optional<std::vector<uint8_t>> LinuxSession::update() {
    auto now = std::chrono::steady_clock::now();
    if (stunEnabled && (now - lastHeartbeatToStun) > std::chrono::seconds(TIME_BETWEEN_HEARTBEATS)) {
        sendHeartbeatToStun<int>(sockFD, stunAddr);
        lastHeartbeatToStun = now;
    }

    while (true) {
        auto [success, data, addr] = recvData<int>(sockFD);
        if (!success) [[likely]] return std::nullopt;  // socket drained

        std::string_view received_str(reinterpret_cast<const char*>(data.data()), data.size());

        if (received_str == "PING") [[unlikely]] {
            static constexpr char pong[] = "PONG";
            sendto(sockFD, pong, sizeof(pong) - 1, 0, (struct sockaddr*)&addr, sizeof(addr));
            addPeerIfNew(addr);
            continue;  // don't return, drain next packet
        }
        if (received_str == "PONG") [[unlikely]] {
            addPeerIfNew(addr);
            continue;
        }

        std::vector<uint8_t> appData(data.size());
        std::memcpy(appData.data(), data.data(), data.size());
        return appData;
    }
}

bool LinuxSession::send(std::span<const uint8_t> data) {
	for (const Peer& peer : peers) {
		sendto(sockFD, data.data(), data.size(), 0, (struct sockaddr*)&peer.sendAddr, sizeof(peer.sendAddr));
	}
	return true;
}

#endif // JUNTOS_UNIX