#include <session_linux.h>

#include <string.h>
#include <iostream>

LinuxSession::LinuxSession() {

}

LinuxSession::~LinuxSession() {
    close(sendSockFD);
    close(recvSockFD);
}

int LinuxSession::createSocket(const char* ip, const int port) {
    int s;
    struct sockaddr_in si;
    
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		std::puts("Error creating socket!\n");
        close(s);
		return 0;
	}

    memset(&si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(port);

	// Convert IP address to binary
    if (inet_pton(AF_INET, ip, &si.sin_addr) <= 0) {
        std::cout << "Error converting IP address to binary, for IP " << ip << " and port " << port << std::endl;
        perror("inet_pton failed");
        return 0;
    }

    if (bind(s, (struct sockaddr*)&si, sizeof(si)) == -1){
		std::puts("Error binding to socket!\n");
        perror("Bind failed");
		return 0;
	}

    return s;
}

bool LinuxSession::init(int portNumber) {
    char ip[20];
    strcpy(ip, "127.0.0.1");

    sendSockFD = createSocket(ip, portNumber);
    recvSockFD = createSocket(ip, portNumber + 1);

    return true;
}