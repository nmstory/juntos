#pragma once

#include <common_juntos.h>

#ifdef JUNTOS_UNIX

#include <session_interface.h>

class LinuxSession : public SessionInterface
{
public:
  LinuxSession();
  ~LinuxSession();

  [[nodiscard]] bool initSessionToStun(const int& portNumber,
                                       const std::string& stunHost,
                                       const int& stunPort) override;
  [[nodiscard]] bool initSessionSolo(const std::string& hostname,
                                     const int& portNumber,
                                     std::optional<std::chrono::milliseconds>
                                         recvTimeout = std::nullopt) override;
  [[nodiscard]] Peer setupPeer(const std::string& destHostname,
                               const int& destPort) override;
  [[nodiscard]] bool send(std::span<const uint8_t> data) override;
  [[nodiscard]] std::optional<std::vector<uint8_t>> update() override;

  Socket getSocketFD() override { return sockFD; };

private:
  // Linux Socket File Descriptor
  int sockFD;
};

#endif
