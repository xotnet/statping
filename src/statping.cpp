// multiplatform checking is remove server server online tool
#include "net.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "color.hpp"

void timer(int* ping, bool* connected) {
	while (!*connected) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		*ping = *ping + 1;
	}
	*ping = 0;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Usage: statping IP PORT";
	}
	while (true) {
		int ping = 0;
		bool connected = false;
		bool success = false;
		std::thread timerThread(timer, &ping, &connected);
		timerThread.detach();
		int conn = connect_net(argv[1], argv[2]);
		if (conn > 0) {
			std::cout << dye::green("Connected ") << dye::light_purple(argv[1]) << ':' << dye::purple(argv[2]) << dye::green(" Ping: ") << dye::light_purple(ping) << dye::purple("ms ") << "\n";
			success = true;
		} else {
			std::cout << dye::red("Connection timed out\n");
		}
		connected = true;
		close_net(conn);
		if (success) {std::this_thread::sleep_for(std::chrono::milliseconds(1000));}
		success = false;
	}
}