#pragma comment(lib, "ws2_32.lib")

#include <client.h>
#include <iostream>
#include <packet.h>
#include <position.h>

int main(int argc, char *argv[]) {
	if (argc < 3) {
		std::puts("Insufficient number of parameters entered. Please define port number.");
		return 1;
	}

	PositionReplicable position;
	Client client;
	client.init(std::stoi(argv[1]), std::stoi(argv[2]), &position);

	position.id = makeID(std::stoi(argv[1]), 1);
	position.position.x = 1.0f;
	position.position.y = 2.0f;

	while (true) {
		if (!client.update()) {
			return 1;
		}
	}

	return 0;
}
