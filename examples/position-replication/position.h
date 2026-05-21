#pragma once

#include <packet.h>
#include <replicable.h>
#include <cstring>
#include <iostream>

struct Position { float x = 0, y = 0; };

struct PositionReplicable : public Replicable {
    std::optional<std::vector<uint8_t>> serialize() override {
        Packet<Position> packet{static_cast<ID>(id), {position.x, position.y} };
        std::vector<uint8_t> buf(sizeof(packet));
        std::memcpy(buf.data(), &packet, sizeof(packet));
        return buf;
    }

    void deserialize(const uint8_t* data, size_t len) override {
        if (len < sizeof(Packet<Position>)) return;
        Packet<Position> packet;
        std::memcpy(&packet, data, sizeof(packet));
        std::cout << "client " << getClientID(packet.id) << " pos: (" << packet.data.x << ", " << packet.data.y << ")\n";
    }

	Position position;
};

