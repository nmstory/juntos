#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <client.h>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::puts("Insufficient number of parameters entered. Please define port number.");
		return 1;
	}

	Client client;
	client.init(std::stoi(argv[1]));

	while(true){
		if(!client.update()) {
			return 1;
		}
	}

	return 0;
}