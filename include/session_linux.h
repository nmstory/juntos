#pragma once

#include <session_interface.h>

class LinuxSession : public SessionInterface {
public:
    LinuxSession();
    ~LinuxSession();

    bool init(int portNumber) override;
private:
    int createSocket(const char* ip, const int port);

    // Linux Socket File Descriptors
    int sendSockFD;
    int recvSockFD;

};