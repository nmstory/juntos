#pragma once

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#include <session_interface.h>

const char STUN_SERVER_IP[] = "127.0.0.1";
const int STUN_SERVER_PORT = 12345;

class WindowsSession : public SessionInterface {
public:
	WindowsSession();
	~WindowsSession();

	bool initSessionToStun(const int& portNumber) override;
	bool initSessionSolo(const std::string& hostname, const int& portNumber) override;
	Peer addPeer(const std::string& destHostname, const int& destPort) override;
	bool update() override;

	Socket getSocketFD() override {
		return sockFD;
	};
private:
	SOCKET socket;
	WSADATA wsaData;
};

#endif