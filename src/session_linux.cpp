#include <session_linux.h>

#include <string.h>
#include <iostream>

LinuxSession::LinuxSession() {

}

LinuxSession::~LinuxSession() {
    close(sendSockFD);
    close(recvSockFD);
}

sockaddr_in LinuxSession::populateAddress(const char* ip, const int port){
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

int LinuxSession::createSocket(const char* ip, sockaddr_in addr) {
    int s;
    
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		std::puts("Error creating socket!\n");
        close(s);
		return 0;
	}

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1){
		std::puts("Error binding to socket!\n");
        perror("Bind failed");
		return -1;
	}

    return s;
}

bool LinuxSession::init(int portNumber) {
    char ip[20];
    strcpy(ip, "127.0.0.1");

    sendAddr = populateAddress(ip, portNumber);
    sendSockFD = createSocket(ip, sendAddr);

    //recvAddr = populateAddress(ip, (portNumber + 1)); manual adjustment below for testing
    // configure a way to have 4 separate sockets, and your in!!!!!!!!!!!
    do {
        recvAddr = populateAddress(ip, (portNumber++));

    }while(recvSockFD = createSocket(ip, recvAddr) == -1);

    return true;
}

bool LinuxSession::sendData(const char* string, const size_t length){
    int byteCount = sendto(sendSockFD, string, length, 0, (struct sockaddr *)&recvAddr, sizeof(recvAddr)); //0 flags

    if (byteCount > 0) {
        std::cout << "message sent successfully" << std::endl;
        return true;
    }
    else {
        perror("Error sending!\n");
        // would want to close socket etc., as we normally do
    }
    
    return false;
}   

bool LinuxSession::recvData(){
    char buffer[200];
    int byteCount = recv(recvSockFD, buffer, 200, 0);
    
    if (byteCount > 0) {
        std::cout << "Message received: " << buffer << std::endl;
    }
    else {
        perror("Error receving!\n");
        // would want to close socket etc., as we normally do
    }
}