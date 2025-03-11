#pragma once

#include <common.h>

class SessionInterface {
public:
    SessionInterface() = default;
    ~SessionInterface() = default;

    /*
        @brief Initialise Session Functionality
        @return true/false whether client initialisation succeeded or failed
    */
   virtual bool init(const int portNumber) = 0;
   virtual bool sendData(const char* string, const size_t length) = 0;
   virtual bool recvData() = 0;
private:
    /*
        @brief Initialise a BSD Socket
        @return true/false whether client initialisation succeeded or failed
    */
    //virtual int createSocket(const char* ip, const int port) = 0;

};