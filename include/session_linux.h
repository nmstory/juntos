#pragma once

#ifdef __linux__

#include <linux_handler.h>
#include <session_interface.h>

const char STUN_SERVER_IP[] = "127.0.0.1";
const int STUN_SERVER_PORT = 12345;

class LinuxSession : public SessionInterface {
public:
    LinuxSession();
    ~LinuxSession();

    bool init(const int portNumber) override;
    bool update() override;
private:
    // Linux Socket File Descriptor and relevant Address
    int sockFD;
    struct sockaddr_in addr;    
};

#endif