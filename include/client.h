#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <span>
#include <string>

#include <common_juntos.h>
#include <session_interface.h>

class Client
{
public:
  Client();
  ~Client();

  [[nodiscard]] bool init(const int port,
                          const std::string& stunHost = DEFAULT_STUN_HOST,
                          const int stunPort = DEFAULT_STUN_PORT);
  [[nodiscard]] bool init(
      const std::string& hostname,
      const int port,
      std::optional<std::chrono::milliseconds> recvTimeout = std::nullopt);
  [[nodiscard]] bool addPeer(const std::string& hostname, const int port);
  [[nodiscard]] bool send(std::span<const uint8_t> data);
  [[nodiscard]] std::optional<std::vector<uint8_t>> update();
  [[nodiscard]] Socket getSocketFD() const;

private:
  std::unique_ptr<SessionInterface> session;
};
