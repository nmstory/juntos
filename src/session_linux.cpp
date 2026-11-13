#include <common_juntos.h>

#ifdef JUNTOS_UNIX

#  include <cstring>
#  include <string_view>

#  include <session_linux.h>

LinuxSession::LinuxSession()
{
  lastHeartbeatToStun = std::chrono::steady_clock::now();
}

LinuxSession::~LinuxSession()
{
  close(sock);
}

bool LinuxSession::initSessionToStun(const int& portNumber,
                                     const std::string& stunHost,
                                     const int& stunPort)
{
  // TODO: if already initialised, inform and back out
  stunEnabled = true;

  localAddr = populateAddress("0.0.0.0", portNumber);
  sock = createSocket<int>(localAddr);

  stunAddr = populateAddress(stunHost.c_str(), stunPort);
  if (stunAddr.sin_family != AF_INET) {
    std::cerr << "Invalid STUN server address: " << stunHost << ":" << stunPort
              << std::endl;
    close(sock);
    return false;
  }

  // Join the server
  static constexpr char joinMessage[] = "JOIN";
  sendto(sock,
         joinMessage,
         sizeof(joinMessage) - 1,
         0,
         (struct sockaddr*)&stunAddr,
         sizeof(stunAddr));

  char buffer[4096];
  socklen_t serverAddrLen = sizeof(stunAddr);
  ssize_t bytesReceived = recvfrom(sock,
                                   buffer,
                                   sizeof(buffer) - 1,
                                   0,
                                   (struct sockaddr*)&stunAddr,
                                   &serverAddrLen);

  if (bytesReceived > 0) {
    std::cout << "Server response: "
              << std::string_view(buffer, static_cast<size_t>(bytesReceived))
              << std::endl;
  } else {
    std::cerr << "No response from server or error occurred" << std::endl;
  }

  // Get the list of clients
  static constexpr char listMessage[] = "LIST:";
  sendto(sock,
         listMessage,
         sizeof(listMessage) - 1,
         0,
         (struct sockaddr*)&stunAddr,
         sizeof(stunAddr));

  bytesReceived = recvfrom(sock,
                           buffer,
                           sizeof(buffer) - 1,
                           0,
                           (struct sockaddr*)&stunAddr,
                           &serverAddrLen);

  // Set socket to non-blocking now the blocking JOIN/LIST handshake is done.
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  if (bytesReceived > 0) {
    std::string_view list(buffer, static_cast<size_t>(bytesReceived));
    if (list == "EMPTY") {
      std::puts("Currently waiting for other clients to connect! Hang on :)");
      return true;
    }
    addPeersFromList(list);
  }

  pingAllPeers();
  return true;
}

bool LinuxSession::initSessionSolo(
    const std::string& localHostname,
    const int& portNumber,
    std::optional<std::chrono::milliseconds> recvTimeout)
{
  // TODO: if already initialised, inform and back out
  localAddr = populateAddress(localHostname.c_str(), portNumber);
  sock = createSocket<int>(localAddr);

  if (recvTimeout) {
    struct timeval tv {};
    tv.tv_sec = recvTimeout->count() / 1000;
    tv.tv_usec = (recvTimeout->count() % 1000) * 1000;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
      std::cerr << "Failed to set SO_RCVTIMEO: " << std::strerror(errno)
                << std::endl;
    }
  }

  return true;
}

#endif  // JUNTOS_UNIX
