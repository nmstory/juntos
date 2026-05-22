#pragma once

#include <../examples/position-replication/config.h>
#include <cstdint>
#include <vector>

using ID = uint32_t;

template<typename T>
struct Packet {
    ID id; // client ID & net object ID
    T data;
};

inline ID makeID(uint32_t clientID, uint32_t localID) {
    return ((clientID & CLIENT_MASK) << LOCAL_BITS) |
           (localID  & LOCAL_MASK);
}

inline uint32_t getClientID(ID id) {
    return (id >> LOCAL_BITS) & CLIENT_MASK;
}

inline uint32_t getObjectID(ID id) {
    return id & LOCAL_MASK;
}