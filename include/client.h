#pragma once

#include <common_juntos.h>

#include <chrono>
#include <memory>
<<<<<<< Updated upstream
=======
<<<<<<< Updated upstream
#include <replicable.h>
=======
#include <optional>
#include <string>
>>>>>>> Stashed changes
>>>>>>> Stashed changes
#include <session_interface.h>

class Client {
public:
	Client();
	~Client();

<<<<<<< Updated upstream
	bool init(const int port);
	bool send(const uint8_t* data, size_t len);
	std::optional<std::vector<uint8_t>> update();
=======
<<<<<<< Updated upstream
	bool init(int port);
	bool init(int clientID, int port, Replicable* replicable);
	bool update();
=======
	bool init(const int port);
	bool init(const std::string& hostname, const int port,
			std::optional<std::chrono::milliseconds> recvTimeout = std::nullopt);
	bool addPeer(const std::string& hostname, const int port);
	bool send(const uint8_t* data, size_t len);
	std::optional<std::vector<uint8_t>> update();
>>>>>>> Stashed changes
>>>>>>> Stashed changes
private:
	std::unique_ptr<SessionInterface> session;
};
