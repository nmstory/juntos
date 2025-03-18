#pragma once

#ifdef _WIN32

#include <common.h>
#include <network_handler.h>
#include <session_interface.h>
#pragma comment(lib, "ws2_32.lib")

const char STUN_SERVER_IP[] = "127.0.0.1";
const int STUN_SERVER_PORT = 12345;

class WindowsSession : public SessionInterface {
public:
	WindowsSession();
	~WindowsSession();

	bool init(const int portNumber) override;
	bool update() override;
private:
	SOCKET socket;
	struct sockaddr_in addr;
	WSADATA wsaData;
};

#endif