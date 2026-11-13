#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include <common_juntos.h>
#include <string.h>

const short TIME_BETWEEN_HEARTBEATS = 5;  // seconds

// Default STUN server endpoint, used when none is supplied at init
const char DEFAULT_STUN_HOST[] = "127.0.0.1";
const int DEFAULT_STUN_PORT = 12345;

/*
        @brief Populate a socket address from a hostname or IP and a port.
   Uses getaddrinfo, so it may block on DNS — keep it out of the hot path.
        @param ip: hostname or IP address
        @param port: port number
        @returns sockaddr_in: Populated sockaddr_in (sin_family == 0 on failure)
*/
sockaddr_in populateAddress(const char* ip, const int& port);

/*
        @brief Build a socket address from a numeric IPv4 string, with no DNS
   lookup and no allocation. Used for addresses received over the wire.
        @param ip: dotted-quad IPv4 address, e.g. "203.0.113.7"
        @param port: port number
        @returns The address, or nullopt if ip isn't a valid IPv4 literal
*/
std::optional<sockaddr_in> parseNumericAddress(std::string_view ip,
                                               uint16_t port);

/*
        @brief Parse "ip:port" (as sent by the STUN server) without DNS
        @returns The address, or nullopt if malformed
*/
std::optional<sockaddr_in> parseEndpoint(std::string_view endpoint);

/*
        @brief Output an error message
        @param error: Error message to output
*/
inline void outputError(const std::string& error)
{
  std::cerr << error << std::endl;
#if _WIN32
  std::cout << WSAGetLastError() << std::endl;
#elif defined(JUNTOS_UNIX)
  std::perror("error");
#endif
}

/*
        @brief Creates a socket
        @param addr: Address to bind the socket to
        @returns: created socket
*/
template<typename T>
T createSocket(sockaddr_in& addr)
{
  T s;

#ifdef _WIN32
  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
#elif defined(JUNTOS_UNIX)
  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
#endif
  {
    std::puts("Error creating socket!\n");
#ifdef _WIN32
    std::cout << "Error initialising socket: " << WSAGetLastError()
              << std::endl;
    WSACleanup();
    closesocket(s);
#elif defined(JUNTOS_UNIX)
    close(s);
#endif
    return 0;
  }

  if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    outputError("Error binding socket");
    return -1;
  }

  return s;
}

struct RecvResult
{
  ssize_t bytes;  // > 0 on success
  sockaddr_in from;
};

/*
        @brief Receive one datagram straight into a caller-owned buffer — no
   intermediate copy and no allocation.
        @param sock: Socket to receive from
        @param buffer: Destination buffer (should hold kMaxUdpPayload bytes)
        @returns Bytes received (<= 0 if nothing was available) and the sender
*/
inline RecvResult recvInto(Socket sock, std::span<uint8_t> buffer)
{
  RecvResult result {0, {}};
  socklen_t addrLen = sizeof(result.from);

  result.bytes = recvfrom(sock,
                          reinterpret_cast<char*>(buffer.data()),
                          static_cast<int>(buffer.size()),
                          0,
                          reinterpret_cast<sockaddr*>(&result.from),
                          &addrLen);

#ifdef _WIN32
  if (result.bytes == SOCKET_ERROR) {
    int err = WSAGetLastError();
    if (err != WSAEWOULDBLOCK && err != WSAETIMEDOUT) {
      std::cerr << "recvfrom error: " << err << std::endl;
    }
  }
#elif defined(JUNTOS_UNIX)
  if (result.bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
    std::perror("recvfrom error");
  }
#endif

  return result;
}

/*
        @brief Send a heartbeat to the STUN server
        @param sockfd: Socket to send the heartbeat from
        @param serverAddr: Address of the STUN server
*/
template<typename T>
void sendHeartbeatToStun(const T& sockfd, const sockaddr_in& serverAddr)
{
  static constexpr char message[] = "HEARTBEAT:";
  sendto(sockfd,
         message,
         static_cast<int>(sizeof(message) - 1),
         0,
         (struct sockaddr*)&serverAddr,
         static_cast<int>(sizeof(serverAddr)));
}
