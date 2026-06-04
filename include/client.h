#pragma once

#include <common_juntos.h>

#include <memory>
#include <session_interface.h>

class Client {
public:
	Client();
	~Client();

	bool init(const int port);
	bool send(const uint8_t* data, size_t len);
	std::optional<std::vector<uint8_t>> update();
private:
	std::unique_ptr<SessionInterface> session;
};
