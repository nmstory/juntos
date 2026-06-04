#ifdef _WIN32

#include <session_windows.h>
#include <cstring>

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

bool WindowsSession::initSessionToStun(const int& portNumber) {
	stunEnabled = true;
	char ip[20];
	strcpy(ip, "127.0.0.1");

	localAddr = populateAddress(ip, portNumber);
	socket = createSocket<SOCKET>(localAddr);

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
				addPeerIfNew(populateAddress(ip, atoi(splitterIndex)));
				splitterIndex = strtok(NULL, ":;");
			}
		}
	}

	// Set socket to non-blocking
	u_long mode = 1; // Non-blocking mode
	if (ioctlsocket(socket, FIONBIO, &mode) != 0) {
		std::cerr << "Failed to set socket to non-blocking mode." << std::endl;
		WSACleanup();
		return 1;
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

bool WindowsSession::initSessionSolo(const std::string& hostname, const int& portNumber, std::optional<std::chrono::milliseconds> recvTimeout) {
	// TODO: if already initialised, inform and back out
	localAddr = populateAddress(hostname.c_str(), portNumber);
	socket = createSocket<SOCKET>(localAddr);

	if (recvTimeout) {
		DWORD timeoutMs = static_cast<DWORD>(recvTimeout->count());
		if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs)) != 0) {
			std::cerr << "Failed to set SO_RCVTIMEO: " << WSAGetLastError() << std::endl;
		}
	}

	return true;
}

Peer WindowsSession::setupPeer(const std::string& destHostname, const int& destPort) {
	sockaddr_in peerAddr = populateAddress(destHostname.c_str(), destPort);
	addPeerIfNew(peerAddr);

	// Bootstrap the handshake so the remote end discovers us via its update() PING handler
	std::string pingMessage = "PING";
	sendto(socket, pingMessage.c_str(), pingMessage.length(), 0, (struct sockaddr*)&peerAddr, sizeof(peerAddr));

	return Peer(peerAddr);
}

std::optional<std::vector<uint8_t>> WindowsSession::update() {
	auto now = std::chrono::steady_clock::now();
	if (stunEnabled && (now - lastHeartbeatToStun) > std::chrono::seconds(TIME_BETWEEN_HEARTBEATS)) {
		sendHeartbeatToStun<SOCKET>(socket, stunAddr);
		lastHeartbeatToStun = now;
	}

	auto [success, data, addr] = recvData<SOCKET>(socket);

	if (success) {
		std::string_view received_str(reinterpret_cast<const char*>(data.data()), data.size());

		if (received_str == "PING") {
			std::string pongMessage = "PONG";
			sendto(socket, pongMessage.c_str(), pongMessage.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
			addPeerIfNew(addr);
			return std::nullopt;
		}
		if (received_str == "PONG") {
			addPeerIfNew(addr);
			return std::nullopt;
		}

		std::vector<uint8_t> appData(data.size());
		std::memcpy(appData.data(), data.data(), data.size());
		return appData;
	}

	return std::nullopt;
}

bool WindowsSession::send(const uint8_t* data, size_t len) {
	for (const Peer& peer : *peers) {
		sendto(socket, reinterpret_cast<const char*>(data), static_cast<int>(len), 0,
		       (struct sockaddr*)&peer.sendAddr, sizeof(peer.sendAddr));
	}
	return true;
}

#endif