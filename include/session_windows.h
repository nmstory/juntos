#pragma once

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")

#include <session_interface.h>

class WindowsSession : public SessionInterface
{
public:
  WindowsSession();
  ~WindowsSession() override;

  [[nodiscard]] bool initSessionToStun(const int& portNumber,
                                       const std::string& stunHost,
                                       const int& stunPort) override;
  [[nodiscard]] bool initSessionSolo(const std::string& hostname,
                                     const int& portNumber,
                                     std::optional<std::chrono::milliseconds>
                                         recvTimeout = std::nullopt) override;

private:
  WSADATA wsaData;
};

#endif
