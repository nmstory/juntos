#pragma once

#include <common.h>
#include <session_interface.h>

class WindowsSession : public SessionInterface {
public:
    WindowsSession();
    ~WindowsSession();

    virtual bool init(const int portNumber) = 0;
    virtual bool sendData(const char* string, const size_t length) = 0;
    virtual bool recvData() = 0;
private:
    //socket socket;

};