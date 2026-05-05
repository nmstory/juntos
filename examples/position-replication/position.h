#pragma once

#include <replicable.h>
#include <cstring>
#include <iostream>

struct PositionReplicable : public Replicable {
	float x = 0.0f;
	float y = 0.0f;

	std::optional<std::vector<uint8_t>> serialize() override {
		std::vector<uint8_t> buf(sizeof(float) * 2);
		std::memcpy(buf.data(), &x, sizeof(float));
		std::memcpy(buf.data() + sizeof(float), &y, sizeof(float));
		return buf;
	}

	void deserialize(const uint8_t* data, size_t len) override {
		if (len < sizeof(float) * 2) return;
		std::memcpy(&x, data, sizeof(float));
		std::memcpy(&y, data + sizeof(float), sizeof(float));
		std::cout << "Received position: (" << x << ", " << y << ")\n";
	}
};
