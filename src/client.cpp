#include <client.h>

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

bool Client::init(int port) {
	session = CreateSession();
	session->initSessionToStun(port);
	return true;
}

bool Client::init(int port, Replicable* rep) {
	replicable = rep;
	session = CreateSession();
	session->initSessionToStun(port);
	return true;
}

bool Client::update() {
	if (replicable) {
		auto payload = replicable->serialize();
		if (payload) {
			session->send(payload->data(), payload->size());
		}
	}

	auto received = session->update();
	if (received && replicable) {
		replicable->deserialize(received->data(), received->size());
	}

	return true;
}
