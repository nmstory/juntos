#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <client.h>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::puts("Insufficient number of parameters entered. Please define port number.");
		return 1;
	}

	Client client;

	int port = std::stoi(argv[1]);
	std::string stunHost = (argc > 2) ? argv[2] : DEFAULT_STUN_HOST;
	int stunPort = (argc > 3) ? std::stoi(argv[3]) : DEFAULT_STUN_PORT;

	client.init(port, stunHost, stunPort);

	// echo
	while (true) {
		if (auto bytes = client.update()) {
			client.send(*bytes);
		}
	}

	return 0;
}