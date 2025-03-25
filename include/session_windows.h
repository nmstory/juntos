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

	bool init(const int& portNumber) override;
	bool update() override;
private:
	SOCKET socket;
	WSADATA wsaData;
};

#endif