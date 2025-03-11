#include <client.h>

Client::Client() {

}

Client::~Client() {

}

// Factory function to inject correct network session dependency, based on platform
extern std::unique_ptr<SessionInterface> CreateSession() {
    #ifdef _WIN32
        return std::make_unique<WindowsSession>();
    #elif __linux__
        return std::make_unique<LinuxSession>();
    #else
        throw std::runtime_error("Unsupported platform");
    #endif
}

bool Client::init(int port) {
	session = CreateSession();

    session->init(port);

    return true;
}

bool Client::update(){
    if(!session){
        return false;
    }

    bool sessionActive = true;

    while (sessionActive){
        session->recvData();

        char buffer[200];
        std::cout << "enter msg to send to server: ";
        if(std::cin.getline(buffer, 200)){
            session->sendData(buffer, 200);
        }   
    }
}



