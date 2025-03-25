#include <network_handler.h>

sockaddr_in populateAddress(const char* ip, const int& port) {
	struct sockaddr_in si;

	memset(&si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(port);

	// Convert IP address to binary
	if (inet_pton(AF_INET, ip, &si.sin_addr) <= 0) {
		std::cout << "Error converting IP address to binary, for IP " << ip << " and port " << port << std::endl;
		perror("inet_pton failed");
	}

	return si;
}
