#pragma once

#include <cstddef>
#include <cstdint>

/*
        Peer-to-peer wire format: every datagram starts with a single MsgType
   byte. Dispatch in update() is a switch on that byte rather than string
   comparisons, and application payloads can never be mistaken for protocol
   messages.

        STUN handshake traffic (JOIN / LIST / HEARTBEAT and their replies)
   stays plain text; it only happens during the blocking init. NEW_PEER is the
   one STUN message that reaches update(), so stun.py prefixes it with the
   NewPeer byte.
*/
enum class MsgType : uint8_t
{
  Ping = 1,  // [Ping]                    hole-punch probe, answered with Pong
  Pong = 2,  // [Pong]                    hole-punch acknowledgement
  NewPeer = 3,  // [NewPeer]"ip:port"     sent by the STUN server
  Data = 4,  // [Data]<payload>           application bytes
};

// Largest payload a single IPv4 UDP datagram can carry
constexpr size_t kMaxUdpPayload = 65507;

// Largest application payload, after the 1-byte header
constexpr size_t kMaxAppPayload = kMaxUdpPayload - 1;
