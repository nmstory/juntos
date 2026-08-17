#ifdef _WIN32

#  include <cstring>

#  include <session_windows.h>

WindowsSession::WindowsSession()
{
  int wsaerr;
  WORD wVersionRequested = MAKEWORD(2, 2);
  wsaerr = WSAStartup(wVersionRequested, &wsaData);
  if (wsaerr != 0) {
    std::cout << "Failed to locate the Winsock dll" << std::endl;
  }
}

WindowsSession::~WindowsSession()
{
  closesocket(socket);
  WSACleanup();
}

bool WindowsSession::initSessionToStun(const int& portNumber,
                                       const std::string& stunHost,
                                       const int& stunPort)
{
  stunEnabled = true;

  localAddr = populateAddress("0.0.0.0", portNumber);
  socket = createSocket<SOCKET>(localAddr);

  stunAddr = populateAddress(stunHost.c_str(), stunPort);
  if (stunAddr.sin_family != AF_INET) {
    std::cerr << "Invalid STUN server address: " << stunHost << ":" << stunPort
              << std::endl;
    closesocket(socket);
    return false;
  }

  // Join the server
  static constexpr char joinMessage[] = "JOIN";
  sendto(socket,
         joinMessage,
         static_cast<int>(sizeof(joinMessage) - 1),
         0,
         (struct sockaddr*)&stunAddr,
         static_cast<int>(sizeof(stunAddr)));

  char buffer[4096];
  socklen_t serverAddrLen = sizeof(stunAddr);
  int bytesReceived = recvfrom(socket,
                               buffer,
                               static_cast<int>(sizeof(buffer) - 1),
                               0,
                               (struct sockaddr*)&stunAddr,
                               &serverAddrLen);

  if (bytesReceived > 0) {
    buffer[bytesReceived] = '\0';
    std::cout << "Server response: " << buffer << std::endl;
  } else {
    std::cerr
        << "No response from server or error occurred. recvfrom returned: "
        << bytesReceived << std::endl;
    int err = WSAGetLastError();
    std::cerr << "recvfrom error: " << err << std::endl;
  }

  // Get the list of clients
  static constexpr char listMessage[] = "LIST:";
  sendto(socket,
         listMessage,
         static_cast<int>(sizeof(listMessage) - 1),
         0,
         (struct sockaddr*)&stunAddr,
         static_cast<int>(sizeof(stunAddr)));

  bytesReceived = recvfrom(socket,
                           buffer,
                           static_cast<int>(sizeof(buffer) - 1),
                           0,
                           (struct sockaddr*)&stunAddr,
                           &serverAddrLen);
  u_long mode = 1;  // Non-blocking mode
  if (ioctlsocket(socket, FIONBIO, &mode) != 0) {
    std::cerr << "Failed to set socket to non-blocking mode." << std::endl;
    WSACleanup();
    return false;
  }

  if (bytesReceived > 0) {
    if (bytesReceived == 5) {
      std::puts("Currently waiting for other clients to connect! Hang on :)");
      return true;
    } else {
      buffer[bytesReceived] = '\0';

      char* ip;
      char* ctx = nullptr;
      char* splitterIndex = strtok_s(buffer, ":;", &ctx);

      while (splitterIndex != NULL) {
        ip = splitterIndex;
        splitterIndex = strtok_s(NULL, ":;", &ctx);
        addPeerIfNew(populateAddress(ip, atoi(splitterIndex)));
        splitterIndex = strtok_s(NULL, ":;", &ctx);
      }
    }
  }

  // ping each client
  static constexpr char pingMessage[] = "PING";
  for (const Peer& peer : peers) {
    int bytesSent = sendto(socket,
                           pingMessage,
                           static_cast<int>(sizeof(pingMessage) - 1),
                           0,
                           (struct sockaddr*)&peer.sendAddr,
                           static_cast<int>(sizeof(peer.sendAddr)));
    if (bytesSent == -1) {
      std::cerr << "Error sending PING to peer "
                << peer.sendAddr.sin_addr.s_addr << ":"
                << ntohs(peer.sendAddr.sin_port) << std::endl;
    }
  }

  return true;
}

bool WindowsSession::initSessionSolo(
    const std::string& hostname,
    const int& portNumber,
    std::optional<std::chrono::milliseconds> recvTimeout)
{
  // TODO: if already initialised, inform and back out
  localAddr = populateAddress(hostname.c_str(), portNumber);
  socket = createSocket<SOCKET>(localAddr);

  if (recvTimeout) {
    DWORD timeoutMs = static_cast<DWORD>(recvTimeout->count());
    if (setsockopt(socket,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeoutMs),
                   sizeof(timeoutMs))
        != 0)
    {
      std::cerr << "Failed to set SO_RCVTIMEO: " << WSAGetLastError()
                << std::endl;
    }
  }

  return true;
}

Peer WindowsSession::setupPeer(const std::string& destHostname,
                               const int& destPort)
{
  sockaddr_in peerAddr = populateAddress(destHostname.c_str(), destPort);
  addPeerIfNew(peerAddr);

  // Bootstrap the handshake so the remote end discovers us via its update()
  // PING handler
  static constexpr char pingMessage[] = "PING";
  sendto(socket,
         pingMessage,
         static_cast<int>(sizeof(pingMessage) - 1),
         0,
         (struct sockaddr*)&peerAddr,
         static_cast<int>(sizeof(peerAddr)));

  return Peer(peerAddr);
}

std::optional<std::vector<uint8_t>> WindowsSession::update()
{
  auto now = std::chrono::steady_clock::now();
  if (stunEnabled
      && (now - lastHeartbeatToStun)
          > std::chrono::seconds(TIME_BETWEEN_HEARTBEATS))
  {
    sendHeartbeatToStun<SOCKET>(socket, stunAddr);
    lastHeartbeatToStun = now;
  }

  auto [success, data, addr] = recvData<SOCKET>(socket);

  if (success) [[unlikely]] {
    std::string_view received_str(reinterpret_cast<const char*>(data.data()),
                                  data.size());

    if (received_str == "PING") [[unlikely]] {
      static constexpr char pongMessage[] = "PONG";
      sendto(socket,
             pongMessage,
             static_cast<int>(sizeof(pongMessage) - 1),
             0,
             (struct sockaddr*)&addr,
             static_cast<int>(sizeof(addr)));
      addPeerIfNew(addr);
      return std::nullopt;
    }
    if (received_str == "PONG") [[unlikely]] {
      addPeerIfNew(addr);
      return std::nullopt;
    }
    static constexpr std::string_view newPeerPrefix = "NEW_PEER:";
    if (received_str.starts_with(newPeerPrefix)) [[unlikely]] {
      std::string_view payload = received_str.substr(newPeerPrefix.size());
      size_t sep = payload.rfind(':');
      if (sep != std::string_view::npos) {
        std::string ip(payload.substr(0, sep));
        int port = std::atoi(std::string(payload.substr(sep + 1)).c_str());
        sockaddr_in peerAddr = populateAddress(ip.c_str(), port);
        addPeerIfNew(peerAddr);

        // Ping the newcomer straight away
        static constexpr char pingMessage[] = "PING";
        sendto(socket,
               pingMessage,
               static_cast<int>(sizeof(pingMessage) - 1),
               0,
               (struct sockaddr*)&peerAddr,
               static_cast<int>(sizeof(peerAddr)));
      }
      return std::nullopt;
    }

    std::vector<uint8_t> appData(data.size());
    std::memcpy(appData.data(), data.data(), data.size());
    return appData;
  }

  return std::nullopt;
}

bool WindowsSession::send(std::span<const uint8_t> data)
{
  for (const Peer& peer : peers) {
    sendto(socket,
           reinterpret_cast<const char*>(data.data()),
           static_cast<int>(data.size()),
           0,
           (struct sockaddr*)&peer.sendAddr,
           sizeof(peer.sendAddr));
  }
  return true;
}

#endif
