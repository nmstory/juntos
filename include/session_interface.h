#pragma once

#include <common.h>
#include <vector>

struct Peer {
	Peer(sockaddr_in rhs) : sendAddr(rhs) { }
	struct sockaddr_in sendAddr;
};

class SessionInterface {
public:
	SessionInterface() = default;
	~SessionInterface() = default;

	/*
		@brief Initialise Session Functionality
		@return true/false whether client initialisation succeeded or failed
	*/
   virtual bool init(const int portNumber) = 0;
   virtual bool update() = 0;
protected:
	std::vector<Peer>* peers;
};