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
	std::unique_ptr<SessionInterface> session = CreateSession();

    session->init(port);

    return true;
}



