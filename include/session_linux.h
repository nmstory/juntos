#pragma once

#include <common_juntos.h>

#ifdef JUNTOS_UNIX

#include <session_interface.h>

class LinuxSession : public SessionInterface
{
public:
  LinuxSession();
  ~LinuxSession() override;

  [[nodiscard]] bool initSessionToStun(const int& portNumber,
                                       const std::string& stunHost,
                                       const int& stunPort) override;
  [[nodiscard]] bool initSessionSolo(const std::string& hostname,
                                     const int& portNumber,
                                     std::optional<std::chrono::milliseconds>
                                         recvTimeout = std::nullopt) override;
};

#endif
