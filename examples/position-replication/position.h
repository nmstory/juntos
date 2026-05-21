#pragma once

#include <packet.h>
#include <replicable.h>
#include <cstring>
#include <iostream>

struct PositionReplicable : public Replicable {
	std::optional<std::vector<uint8_t>> serialize() override {
		std::vector<uint8_t> buf(POSITION_PACKET_SIZE);
		std::memcpy(buf.data(), &id, sizeof(int));
		std::memcpy(buf.data() + sizeof(int), &x, sizeof(float));
		std::memcpy(buf.data() + sizeof(int) + sizeof(float), &y, sizeof(float));
		return buf;
	}

	void deserialize(const uint8_t* data, size_t len) override {
		if (len < POSITION_PACKET_SIZE) return;
		uint8_t recv_id;
		std::memcpy(&recv_id, data, sizeof(int));
		std::memcpy(&x, data + sizeof(int), sizeof(float));
		std::memcpy(&y, data + sizeof(int) + sizeof(float), sizeof(float));
		std::cout << "Received from client " << getClientID(recv_id) << " position id: " << getObjectID(recv_id) << " at: (" << x << ", " << y << ")\n";
	}

	float x = 0.0f;
	float y = 0.0f;

	const int POSITION_PACKET_SIZE = sizeof(int) + (sizeof(float) * 2);
};
