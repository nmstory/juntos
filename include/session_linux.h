#pragma once

#ifdef __linux__

#include <session_interface.h>

const char STUN_SERVER_IP[] = "127.0.0.1";
const int STUN_SERVER_PORT = 12345;

class LinuxSession : public SessionInterface {
public:
	LinuxSession();
	~LinuxSession();

	bool initSessionToStun(const int& portNumber) override;
	bool initSessionSolo(const std::string& hostname, const int& portNumber) override;
	Peer addPeer(const std::string& destHostname, const int& destPort) override;
	bool update() override;

	Socket getSocketFD() override {
		return sockFD;
	};
private:
	// Linux Socket File Descriptor
	int sockFD;
};

#endif