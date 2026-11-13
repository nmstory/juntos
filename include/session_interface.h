#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <common_juntos.h>
#include <network_handler.h>
#include <protocol.h>

struct Peer
{
  Peer(const sockaddr_in& rhs)
      : sendAddr(rhs)
  {
  }

  Peer(const Peer& other) = default;

  Peer(Peer&& peer)
      : sendAddr(peer.sendAddr)
  {
  }

  Peer& operator=(Peer&& other) noexcept
  {
    sendAddr = other.sendAddr;
    return *this;
  }
  struct sockaddr_in sendAddr;
};

/*
        Platform-independent session. The hot path (update / send /
   waitForData) lives here once; platform subclasses only handle socket setup,
   the STUN handshake and teardown.
*/
class SessionInterface
{
public:
  SessionInterface() = default;
  virtual ~SessionInterface() = default;

  SessionInterface(const SessionInterface&) = delete;
  SessionInterface& operator=(const SessionInterface&) = delete;

  /*
          @brief Initialise session functionality without contacting the STUN
     server
          @param hostname The hostname to bind the local socket to
          @param portNumber The port number to use for the local socket
          @param recvTimeout Optional receive timeout
          @return True/False whether client initialisation succeeded or failed
  */
  [[nodiscard]] virtual bool initSessionSolo(
      const std::string& hostname,
      const int& portNumber,
      std::optional<std::chrono::milliseconds> recvTimeout = std::nullopt) = 0;

  /*
          @brief Initialise session functionality by contacting the STUN server
          @param portNumber The port number to use for the local socket
          @param stunHost The STUN server hostname/IP to contact
          @param stunPort The STUN server port
          @return True/False whether client initialisation succeeded or failed
  */
  [[nodiscard]] virtual bool initSessionToStun(
      const int& portNumber,
      const std::string& stunHost = DEFAULT_STUN_HOST,
      const int& stunPort = DEFAULT_STUN_PORT) = 0;

  /*
          @brief Register a peer and send it a PING to start the handshake
          @param destHostname The hostname of the peer to add
          @param destPort The port number of the peer to add
          @return The added peer
  */
  [[nodiscard]] Peer setupPeer(const std::string& destHostname,
                               const int& destPort);

  /*
          @brief Handle protocol traffic and return the next application
     payload. Drains every pending packet until it finds application data or
     the socket is empty.
          @return A view of the received bytes, or nullopt if nothing (or only
     protocol messages) arrived. The view points into a buffer owned by this
     session and is only valid until the next update() call — copy it if you
     need to keep it.
  */
  [[nodiscard]] std::optional<std::span<const uint8_t>> update();

  /*
          @brief Send bytes to all known peers. Allocation-free: the 1-byte
     header and payload are framed in a reused buffer.
          @return false if the payload is too large or any send failed
  */
  [[nodiscard]] bool send(std::span<const uint8_t> data);

  /*
          @brief Block until the socket has data to read, or the timeout
     expires. Lets callers sleep in the kernel instead of busy-polling update().
          @return true if data is ready to be read
  */
  [[nodiscard]] bool waitForData(std::chrono::milliseconds timeout) const;

  /*
          @brief Get the socket file descriptor
  */
  [[nodiscard]] Socket getSocketFD() const { return sock; }

  /*
          @brief Number of currently known peers (cheap; no formatting)
  */
  [[nodiscard]] size_t peerCount() const { return peers.size(); }

  /*
          @brief Human-readable "ip:port" for each currently known peer
          @return One string per peer, in discovery order
  */
  [[nodiscard]] std::vector<std::string> peerAddresses() const;

protected:
  /*
          @brief Add a peer to the peer list, skipping it if an identical
     address is already known
  */
  void addPeerIfNew(const sockaddr_in& addr);

  // React to a non-Data packet (Ping / Pong / NewPeer)
  void handleControl(std::span<const uint8_t> packet, const sockaddr_in& from);

  /*
          @brief Blocking receive of the STUN server's next reply during init.
     Peer control packets that arrive first are handled, not mistaken for the
     reply.
          @return The reply text (a view into the receive buffer), or nullopt
     on error / timeout
  */
  std::optional<std::string_view> recvStunReply();

  // Send a single-byte protocol message (Ping / Pong) to one address
  void sendControl(MsgType type, const sockaddr_in& to) const;

  // Parse the STUN server's "ip:port;ip:port;" guest list and add each peer
  void addPeersFromList(std::string_view list);

  // PING every known peer to open NAT mappings
  void pingAllPeers() const;

  Socket sock {};
  std::vector<Peer> peers;  // Flat, contiguous; tiny N so linear scan wins
  std::chrono::time_point<std::chrono::steady_clock>
      lastHeartbeatToStun;  // Time of last heartbeat sent to the STUN server
  bool stunEnabled =
      false;  // Whether this session was initialised against the STUN server
  struct sockaddr_in localAddr {};
  struct sockaddr_in stunAddr {};

private:
  // Reused for every receive; update() hands out views into it
  std::array<uint8_t, kMaxUdpPayload> recvBuffer;
  // Reused for framing outgoing Data packets
  std::array<uint8_t, kMaxUdpPayload> sendBuffer;
};
