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

  /*
          @brief Process incoming traffic and return the next application
     payload, if any.
          @return A view into the client's receive buffer, valid only until
     the next call to update(). Copy the bytes if you need to keep them.
  */
  [[nodiscard]] std::optional<std::span<const uint8_t>> update();

  /*
          @brief Sleep until data arrives or the timeout expires, instead of
     busy-polling update()
          @return true if data is ready
  */
  [[nodiscard]] bool waitForData(std::chrono::milliseconds timeout) const;

  [[nodiscard]] size_t peerCount() const;
  [[nodiscard]] std::vector<std::string> peerAddresses() const;
  [[nodiscard]] Socket getSocketFD() const;

private:
  std::unique_ptr<SessionInterface> session;
};
