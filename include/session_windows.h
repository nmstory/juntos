#pragma once

#include <common.h>
#include <session_interface.h>

class WindowsSession : public SessionInterface {
public:
    WindowsSession();
    ~WindowsSession();

    bool init(int portNumber) override;
private:
    //socket socket;

};