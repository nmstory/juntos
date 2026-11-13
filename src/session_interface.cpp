#include <session_interface.h>

#include <cstring>

#ifdef JUNTOS_UNIX
#  include <poll.h>
#endif

Peer SessionInterface::setupPeer(const std::string& destHostname,
                                 const int& destPort)
{
  sockaddr_in peerAddr = populateAddress(destHostname.c_str(), destPort);
  addPeerIfNew(peerAddr);

  // Bootstrap the handshake so the remote end discovers us via its update()
  // Ping handler
  sendControl(MsgType::Ping, peerAddr);

  return Peer(peerAddr);
}

std::optional<std::span<const uint8_t>> SessionInterface::update()
{
  if (stunEnabled) {
    auto now = std::chrono::steady_clock::now();
    if (now - lastHeartbeatToStun
        > std::chrono::seconds(TIME_BETWEEN_HEARTBEATS))
    {
      sendHeartbeatToStun(sock, stunAddr);
      lastHeartbeatToStun = now;
    }
  }

  while (true) {
    auto [bytes, from] = recvInto(sock, recvBuffer);
    if (bytes <= 0) [[likely]] {
      return std::nullopt;  // socket drained
    }

    const std::span<const uint8_t> packet(recvBuffer.data(),
                                          static_cast<size_t>(bytes));

    if (static_cast<MsgType>(packet[0]) == MsgType::Data) [[likely]] {
      return packet.subspan(1);
    }
    handleControl(packet, from);  // then keep draining
  }
}

void SessionInterface::handleControl(std::span<const uint8_t> packet,
                                     const sockaddr_in& from)
{
  switch (static_cast<MsgType>(packet[0])) {
    case MsgType::Ping:
      sendControl(MsgType::Pong, from);
      addPeerIfNew(from);
      break;

    case MsgType::Pong:
      addPeerIfNew(from);
      break;

    case MsgType::NewPeer: {
      std::string_view endpoint(
          reinterpret_cast<const char*>(packet.data() + 1), packet.size() - 1);
      if (auto peerAddr = parseEndpoint(endpoint)) {
        addPeerIfNew(*peerAddr);
        sendControl(MsgType::Ping, *peerAddr);  // ping the newcomer now
      }
      break;
    }

    default:
      break;  // Data handled by caller; unknown / stray STUN text: ignore
  }
}

std::optional<std::string_view> SessionInterface::recvStunReply()
{
  // Bounded so a flood of peer packets can't wedge init forever
  for (int attempts = 0; attempts < 64; ++attempts) {
    auto [bytes, from] = recvInto(sock, recvBuffer);
    if (bytes <= 0) {
      return std::nullopt;  // error / timeout
    }

    const std::span<const uint8_t> packet(recvBuffer.data(),
                                          static_cast<size_t>(bytes));

    if (from.sin_addr.s_addr == stunAddr.sin_addr.s_addr
        && from.sin_port == stunAddr.sin_port)
    {
      return std::string_view(reinterpret_cast<const char*>(packet.data()),
                              packet.size());
    }

    // A peer that heard about us via NEW_PEER can PING before the STUN
    // server's reply arrives; answer it rather than mistaking it for the reply
    handleControl(packet, from);
  }
  return std::nullopt;
}

bool SessionInterface::send(std::span<const uint8_t> data)
{
  if (data.size() > kMaxAppPayload) {
    return false;
  }

  // Build header + payload once in a reused buffer, then plain sendto() per
  // peer. Measured: for game-sized packets this beats scatter-gather
  // sendmsg/WSASendTo, whose iovec handling costs more than a small memcpy.
  sendBuffer[0] = static_cast<uint8_t>(MsgType::Data);
  if (!data.empty()) {
    std::memcpy(sendBuffer.data() + 1, data.data(), data.size());
  }
  const int length = static_cast<int>(data.size() + 1);

  bool ok = true;
  for (const Peer& peer : peers) {
    if (sendto(sock,
               reinterpret_cast<const char*>(sendBuffer.data()),
               length,
               0,
               reinterpret_cast<const sockaddr*>(&peer.sendAddr),
               static_cast<int>(sizeof(peer.sendAddr)))
        < 0)
    {
      ok = false;
    }
  }

  return ok;
}

bool SessionInterface::waitForData(std::chrono::milliseconds timeout) const
{
#ifdef _WIN32
  WSAPOLLFD pfd {};
  pfd.fd = sock;
  pfd.events = POLLRDNORM;
  return WSAPoll(&pfd, 1, static_cast<INT>(timeout.count())) > 0;
#else
  pollfd pfd {};
  pfd.fd = sock;
  pfd.events = POLLIN;
  return poll(&pfd, 1, static_cast<int>(timeout.count())) > 0;
#endif
}

std::vector<std::string> SessionInterface::peerAddresses() const
{
  std::vector<std::string> out;
  out.reserve(peers.size());
  for (const Peer& peer : peers) {
    char ip[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &peer.sendAddr.sin_addr, ip, sizeof(ip));
    out.emplace_back(std::string(ip) + ":"
                     + std::to_string(ntohs(peer.sendAddr.sin_port)));
  }
  return out;
}

void SessionInterface::addPeerIfNew(const sockaddr_in& addr)
{
  for (const Peer& peer : peers) {
    if (peer.sendAddr.sin_addr.s_addr == addr.sin_addr.s_addr
        && peer.sendAddr.sin_port == addr.sin_port)
    {
      return;
    }
  }
  peers.push_back(Peer(addr));
}

void SessionInterface::sendControl(MsgType type, const sockaddr_in& to) const
{
  const char byte = static_cast<char>(type);
  sendto(sock,
         &byte,
         1,
         0,
         reinterpret_cast<const sockaddr*>(&to),
         static_cast<int>(sizeof(to)));
}

void SessionInterface::addPeersFromList(std::string_view list)
{
  while (!list.empty()) {
    size_t end = list.find(';');
    std::string_view entry = list.substr(0, end);
    if (auto addr = parseEndpoint(entry)) {
      addPeerIfNew(*addr);
    }
    if (end == std::string_view::npos) {
      break;
    }
    list.remove_prefix(end + 1);
  }
}

void SessionInterface::pingAllPeers() const
{
  for (const Peer& peer : peers) {
    sendControl(MsgType::Ping, peer.sendAddr);
  }
}
