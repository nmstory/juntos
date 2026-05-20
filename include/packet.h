#include <../examples/position-replication/config.h>
#include <cstdint>
#include <vector>

using NetObjectID = uint32_t;

#pragma pack(push, 1)
struct PacketHeader { // packet structure: [objectID][type][size][payload bytes]
    NetObjectID objectID;
    uint8_t type;
    uint16_t size;
};
#pragma pack(pop)

template<typename T>
struct Packet {
    NetObjectID objectID;
    T data;
};

inline NetObjectID makeNetObjectID(uint32_t clientID, uint32_t localID) {
    return ((clientID & CLIENT_MASK) << LOCAL_BITS) |
           (localID  & LOCAL_MASK);
}

inline uint32_t getClientID(NetObjectID id) {
    return (id >> LOCAL_BITS) & CLIENT_MASK;
}

inline uint32_t getLocalID(NetObjectID id) {
    return id & LOCAL_MASK;
}