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

	[[nodiscard]] bool initSessionToStun(const int& portNumber) override;
	[[nodiscard]] bool initSessionSolo(const std::string& hostname, const int& portNumber, std::optional<std::chrono::milliseconds> recvTimeout = std::nullopt) override;
	[[nodiscard]] Peer setupPeer(const std::string& destHostname, const int& destPort) override;
	[[nodiscard]] bool send(std::span<const uint8_t> data) override;
	[[nodiscard]] std::optional<std::vector<uint8_t>> update() override;

	Socket getSocketFD() override {
		return socket;
	};
private:
	SOCKET socket;
	WSADATA wsaData;
};

#endif