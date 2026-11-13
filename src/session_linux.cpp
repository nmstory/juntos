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

  // Set socket to non-blocking now the blocking JOIN/LIST handshake is done.
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

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
