#include <client.h>

#include <session_linux.h>
#include <session_windows.h>

Client::Client() {

}

Client::~Client() {

}

// Factory function to inject correct network session dependency, based on platform
extern std::unique_ptr<SessionInterface> CreateSession() {
	#ifdef _WIN32
		return std::make_unique<WindowsSession>();
	#elif defined(JUNTOS_UNIX)
		return std::make_unique<LinuxSession>();
	#else
		throw std::runtime_error("Unsupported platform");
	#endif
}

bool Client::init(const int port) {
	session = CreateSession();
	return session->initSessionToStun(port);
}

bool Client::init(const std::string& hostname, const int port,
		std::optional<std::chrono::milliseconds> recvTimeout) {
	session = CreateSession();
	return session->initSessionSolo(hostname, port, recvTimeout);
}

bool Client::addPeer(const std::string& hostname, const int port) {
	session->setupPeer(hostname, port);
	return true;
}

bool Client::send(const uint8_t* data, size_t len) {
    return session->send(data, len);
}

std::optional<std::vector<uint8_t>> Client::update() {
	return session->update();
}
