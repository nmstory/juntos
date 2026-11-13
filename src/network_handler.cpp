#ifndef _WIN32
#  include <netdb.h>
#endif
#include <charconv>

#include <network_handler.h>

sockaddr_in populateAddress(const char* host, const int& port)
{
  struct addrinfo hints {}, *res = nullptr;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  std::string port_str = std::to_string(port);
  int err = getaddrinfo(host, port_str.c_str(), &hints, &res);
  if (err != 0 || !res) {
    outputError("getaddrinfo failed for host " + std::string(host) + ": "
                + gai_strerror(err));
    sockaddr_in empty {};
    return empty;
  }

  sockaddr_in addr = *(reinterpret_cast<sockaddr_in*>(res->ai_addr));
  freeaddrinfo(res);
  return addr;
}

std::optional<sockaddr_in> parseNumericAddress(std::string_view ip,
                                               uint16_t port)
{
  // inet_pton needs a NUL-terminated string; "255.255.255.255" is 15 chars
  char buf[16];
  if (ip.empty() || ip.size() >= sizeof(buf)) {
    return std::nullopt;
  }
  ip.copy(buf, ip.size());
  buf[ip.size()] = '\0';

  sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, buf, &addr.sin_addr) != 1) {
    return std::nullopt;
  }
  return addr;
}

std::optional<sockaddr_in> parseEndpoint(std::string_view endpoint)
{
  size_t sep = endpoint.rfind(':');
  if (sep == std::string_view::npos) {
    return std::nullopt;
  }

  std::string_view portStr = endpoint.substr(sep + 1);
  uint16_t port = 0;
  auto [end, ec] =
      std::from_chars(portStr.data(), portStr.data() + portStr.size(), port);
  if (ec != std::errc {} || end != portStr.data() + portStr.size()) {
    return std::nullopt;
  }

  return parseNumericAddress(endpoint.substr(0, sep), port);
}
