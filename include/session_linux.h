#pragma once

#include <session_interface.h>

class LinuxSession : public SessionInterface {
public:
    LinuxSession();
    ~LinuxSession();

    bool init(const int portNumber) override;
    bool sendData(const char* string, const size_t length) override;
    bool recvData() override;
    
private:
    sockaddr_in populateAddress(const char* ip, const int port);
    int createSocket(const char* ip, sockaddr_in addr);

    // Linux Socket File Descriptors
    int sendSockFD;
    struct sockaddr_in sendAddr;
    int recvSockFD;
    struct sockaddr_in recvAddr;
    
};