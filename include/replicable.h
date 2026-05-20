#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

class Replicable {
public:
	virtual std::optional<std::vector<uint8_t>> serialize() = 0;
	virtual void deserialize(const uint8_t* data, size_t len) = 0;
	virtual ~Replicable() = default;

	int id = 0; // Unique identifier for the replicable object
};
