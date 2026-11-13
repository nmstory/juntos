#pragma comment(lib, "ws2_32.lib")

#include <client.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <span>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::puts("Insufficient number of parameters entered. Please define port number.");
		return 1;
	}

	Client client;

	int port = std::stoi(argv[1]);
	std::string stunHost = (argc > 2) ? argv[2] : DEFAULT_STUN_HOST;
	int stunPort = (argc > 3) ? std::stoi(argv[3]) : DEFAULT_STUN_PORT;

	if (!client.init(port, stunHost, stunPort)) {
		std::puts("Failed to initialise client.");
		return 1;
	}

	// Read typed lines on a background thread and hand them to the main loop via a queue.
	std::mutex outboxMutex;
	std::queue<std::string> outbox;
	std::thread([&] {
		std::string line;
		while (std::getline(std::cin, line)) {
			std::lock_guard<std::mutex> lock(outboxMutex);
			outbox.push(std::move(line));
		}
	}).detach();

	std::puts("Connected. Type a message and press enter to send it to all peers.");

	size_t announcedPeers = 0;
	while (true) {
		while (auto bytes = client.update()) {
			std::cout << "Peer: " << std::string(bytes->begin(), bytes->end())
			          << std::endl;
		}
		
		// peerCount() is cheap; only format addresses when someone new shows up.
		if (client.peerCount() != announcedPeers) {
			auto peers = client.peerAddresses();
			for (size_t i = announcedPeers; i < peers.size(); ++i) {
				std::cout << "Peer discovered: " << peers[i] << std::endl;
			}
			announcedPeers = peers.size();
		}

		// Send everything typed since the last iteration to every peer.
		std::queue<std::string> pending;
		{
			std::lock_guard<std::mutex> lock(outboxMutex);
			std::swap(pending, outbox);
		}
		for (; !pending.empty(); pending.pop()) {
			const std::string &msg = pending.front();
			(void)client.send(std::span<const uint8_t>(
			    reinterpret_cast<const uint8_t *>(msg.data()), msg.size()));
		}

		// Sleep in the kernel until a packet arrives (wakes immediately) or 10ms
		// pass, so typed lines still get sent promptly.
		(void)client.waitForData(std::chrono::milliseconds(10));
	}

	return 0;
}
