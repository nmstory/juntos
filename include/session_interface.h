#pragma once

#include <common.h>

#include <chrono>
#include <iostream>
#include <string_view>
#include <thread>
#include <vector>

#include <network_handler.h>
#include <session_interface.h>

struct Peer {
	Peer(const sockaddr_in& rhs) : sendAddr(rhs) { }
	struct sockaddr_in sendAddr;
};

class SessionInterface {
public:
	SessionInterface() = default;
	~SessionInterface() = default;

	/*
		@brief Initialise session functionality
		@return True/False whether client initialisation succeeded or failed
	*/
	virtual bool init(const int& portNumber) = 0;

	/*
		@brief Update the Session
		@return True/False whether client updated successfully or not
	*/
	virtual bool update() = 0;
protected:
	std::vector<Peer>* peers;
	std::chrono::time_point<std::chrono::steady_clock> lastHeartbeatToStun; // Time of last heartbeat sent to the STUN server
	struct sockaddr_in localAddr;
	struct sockaddr_in stunAddr;
};