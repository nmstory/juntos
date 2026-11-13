#ifdef _WIN32

#  include <cstring>
#  include <string_view>

#  include <session_windows.h>

WindowsSession::WindowsSession()
{
  lastHeartbeatToStun = std::chrono::steady_clock::now();

  WORD wVersionRequested = MAKEWORD(2, 2);
  int wsaerr = WSAStartup(wVersionRequested, &wsaData);
  if (wsaerr != 0) {
    std::cout << "Failed to locate the Winsock dll" << std::endl;
  }
}

WindowsSession::~WindowsSession()
{
  closesocket(sock);
  WSACleanup();
}

bool WindowsSession::initSessionToStun(const int& portNumber,
                                       const std::string& stunHost,
                                       const int& stunPort)
{
  stunEnabled = true;

  localAddr = populateAddress("0.0.0.0", portNumber);
  sock = createSocket<SOCKET>(localAddr);

  stunAddr = populateAddress(stunHost.c_str(), stunPort);
  if (stunAddr.sin_family != AF_INET) {
    std::cerr << "Invalid STUN server address: " << stunHost << ":" << stunPort
              << std::endl;
    closesocket(sock);
    return false;
  }

  // Join the server
  static constexpr char joinMessage[] = "JOIN";
  sendto(sock,
         joinMessage,
         static_cast<int>(sizeof(joinMessage) - 1),
         0,
         (struct sockaddr*)&stunAddr,
         static_cast<int>(sizeof(stunAddr)));

  if (auto reply = recvStunReply()) {
    std::cout << "Server response: " << *reply << std::endl;
  } else {
    std::cerr << "No response from server or error occurred" << std::endl;
  }

  // Get the list of clients
  static constexpr char listMessage[] = "LIST:";
  sendto(sock,
         listMessage,
         static_cast<int>(sizeof(listMessage) - 1),
         0,
         (struct sockaddr*)&stunAddr,
         static_cast<int>(sizeof(stunAddr)));

  // Parse the guest list before switching to non-blocking mode
  if (auto list = recvStunReply()) {
    if (*list == "EMPTY") {
      std::puts("Currently waiting for other clients to connect! Hang on :)");
    } else {
      addPeersFromList(*list);
    }
  }

  u_long mode = 1;  // Non-blocking mode
  if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
    std::cerr << "Failed to set socket to non-blocking mode." << std::endl;
    WSACleanup();
    return false;
  }

  pingAllPeers();
  return true;
}

bool WindowsSession::initSessionSolo(
    const std::string& hostname,
    const int& portNumber,
    std::optional<std::chrono::milliseconds> recvTimeout)
{
  // TODO: if already initialised, inform and back out
  localAddr = populateAddress(hostname.c_str(), portNumber);
  sock = createSocket<SOCKET>(localAddr);

  if (recvTimeout) {
    DWORD timeoutMs = static_cast<DWORD>(recvTimeout->count());
    if (setsockopt(sock,
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

#endif
