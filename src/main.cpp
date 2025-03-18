#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <client.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::puts("Insufficient number of parameters entered. Please define port number.");
	}

	Client client;
	client.init(std::stoi(argv[1]));

	while(true){
		client.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return 0;
}