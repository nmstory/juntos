#pragma once

#ifdef _WIN32

#include <common.h>
#include <session_interface.h>
#pragma comment(lib, "ws2_32.lib")

class WindowsSession : public SessionInterface {
public:
    WindowsSession();
    ~WindowsSession();

    bool init(const int portNumber) override;
    bool update() override;
private:
    SOCKET socket;
    struct sockaddr_in addr;
};

#endif