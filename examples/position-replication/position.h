#pragma once

#include <packet.h>
#include <replicable.h>
#include <cstring>
#include <iostream>

struct Position { float x = 0, y = 0; };

struct PositionReplicable : public ReplicableT<PositionReplicable> {
    Position position;

    template<typename Stream>
    bool process_impl(Stream& s) {
        if (!process_pod(s, position.x)) return false;
        if (!process_pod(s, position.y)) return false;
        return true;
    }
};
