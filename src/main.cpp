#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <client.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::puts("Insufficient number of parameters entered. Please define port number.");
    }

    Client client;
	client.init(std::stoi(argv[1]));

    return 0;
}