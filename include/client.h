#pragma once

#include <memory>

#include <common.h>

#include <session_interface.h>
#include <session_linux.h>
#include <session_windows.h>

class Client {
public:
	Client();
	~Client();

	/*
		@brief Initialise Client Functionality
		@return true/false whether client initialisation succeeded or failed
	*/
	bool init(int port);
	bool update();
private:
	std::unique_ptr<SessionInterface> session;
};