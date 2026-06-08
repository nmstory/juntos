#pragma once

#include <common_juntos.h>

#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <session_interface.h>

class Client {
public:
	Client();
	~Client();

	bool init(const int port);
	bool init(const std::string& hostname, const int port,
			std::optional<std::chrono::milliseconds> recvTimeout = std::nullopt);
	bool addPeer(const std::string& hostname, const int port);
	bool send(std::span<const uint8_t> data);
	std::optional<std::vector<uint8_t>> update();
	Socket getSocketFD() const;
private:
	std::unique_ptr<SessionInterface> session;
};
