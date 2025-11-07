#include <netdb.h>
#include <network_handler.h>

sockaddr_in populateAddress(const char* host, const int& port) {
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    std::string port_str = std::to_string(port);
    int err = getaddrinfo(host, port_str.c_str(), &hints, &res);
    if (err != 0 || !res) {
        outputError("getaddrinfo failed for host " + std::string(host) + ": " + gai_strerror(err));
        sockaddr_in empty{};
        return empty;
    }

    sockaddr_in addr = *(reinterpret_cast<sockaddr_in*>(res->ai_addr));
    freeaddrinfo(res);
    return addr;
}
