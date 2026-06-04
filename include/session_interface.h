#pragma once

#include <common_juntos.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string_view>
#include <thread>
#include <vector>

#include <network_handler.h>

struct Peer {
	Peer(const sockaddr_in& rhs) : sendAddr(rhs) { }
    Peer(const Peer& other) = default;
    Peer(Peer&& peer) : sendAddr(peer.sendAddr) { }
	Peer& operator=(Peer&& other) noexcept { sendAddr = other.sendAddr; return *this; }
    struct sockaddr_in sendAddr;
};

class SessionInterface {
public:
	SessionInterface() = default;
	virtual ~SessionInterface() = default;
	/*
		@brief Initialise session functionality without contacting the STUN server
		@param hostname The hostname to bind the local socket to
		@param portNumber The port number to use for the local socket
		@param recvTimeout Optional receive timeout
		@return True/False whether client initialisation succeeded or failed
	*/
	virtual bool initSessionSolo(const std::string& hostname, const int& portNumber,
											std::optional<std::chrono::milliseconds> recvTimeout = std::nullopt) = 0;

	/*
		@brief Initialise session functionality by contacting the STUN server
		@param portNumber The port number to use for the local socket
		@return True/False whether client initialisation succeeded or failed
	*/
	virtual bool initSessionToStun(const int& portNumber) = 0;

	/*
		@brief Create a Peer by configuring it's low-level data
		@param destHostname The hostname of the peer to add
		@param destPort The port number of the peer to add
		@return The added peer
	*/
	virtual Peer setupPeer(const std::string& destHostname, const int& destPort) = 0;

	/*
		@brief Update the Session — handle protocol traffic and return any application-level data received
		@return Received application bytes, or nullopt if nothing (or only protocol messages) arrived
	*/
	virtual std::optional<std::vector<uint8_t>> update() = 0;

	/*
		@brief Send bytes to all known peers
	*/
	virtual bool send(const uint8_t* data, size_t len) = 0;

	/*
		@brief Get the socket file descriptor
		@return Socket file descriptor
	*/
	virtual Socket getSocketFD() = 0;
protected:
	/*
		@brief Add a peer to the peer list, skipping it if an identical address is already known
		@param addr The peer's send address
	*/
	void addPeerIfNew(const sockaddr_in& addr) {
		for (const Peer& peer : *peers) {
			if (peer.sendAddr.sin_addr.s_addr == addr.sin_addr.s_addr &&
				peer.sendAddr.sin_port == addr.sin_port) {
				return;
			}
		}
		peers->push_back(Peer(addr));
	}

	std::vector<Peer>* peers; // TODO: legacy, to be removed
	std::chrono::time_point<std::chrono::steady_clock> lastHeartbeatToStun; // Time of last heartbeat sent to the STUN server
	bool stunEnabled = false; // Whether this session was initialised against the STUN server
	struct sockaddr_in localAddr;
	struct sockaddr_in stunAddr;
};