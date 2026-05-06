#pragma once

#include <common_juntos.h>

#include <memory>
#include <replicable.h>
#include <session_interface.h>
#include <session_linux.h>
#include <session_windows.h>

class Client {
public:
	Client();
	~Client();


	bool init(int port);
	bool init(int port, Replicable* replicable);
	bool update();
private:
	std::unique_ptr<SessionInterface> session;
	Replicable* replicable = nullptr;
};
