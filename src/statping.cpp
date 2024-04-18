// multiplatform checking is remove server server online tool
#ifdef __WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <cstring>
#endif
#include "color.hpp"
#include <iostream>
#include <chrono>
#include <thread>

void connect_net(const char* ip, const char* port, int* connRet, const unsigned short int protocol) { // 0 for TCP | 1 for UDP
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int conn = 0;
	if (protocol == 0) {conn = socket(AF_INET, SOCK_STREAM, 0);}
	else if (protocol == 1) {conn = socket(AF_INET, SOCK_DGRAM, 0);}
	const int enable = 1;
	setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = inet_addr(ip);
	connect(conn, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	*connRet = conn;
}

int close_net(int conn) {
	#ifdef __WIN32
		return closesocket(conn);
	#elif __linux__
		return close(conn);
	#endif
}

void timer(int* ping, bool* connected) {
	while (!*connected) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		*ping = *ping + 1;
	}
	*ping = 0;
}

void timeOutCheck(int howLongMS, bool* timedOut) {
	std::this_thread::sleep_for(std::chrono::milliseconds(howLongMS));
	*timedOut = true;
}

int connectWithTimeOut(int timeoutMS, char* ip, char* port) {
	bool timedOut = false;
	int passed = 0;
	int conn = -1;
	std::thread timeOutCheckThread(timeOutCheck, timeoutMS, &timedOut);
	timeOutCheckThread.detach();
	std::thread connectThread(connect_net, ip, port, &conn, 0);
	connectThread.detach();
	while (!timedOut && passed<timeoutMS && conn <= 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		passed = passed+10;
	}
	return conn;
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
		int conn = connectWithTimeOut(2000, argv[1], argv[2]);
		if (conn > 0) {
			std::cout << dye::green("Connected ") << dye::light_purple(argv[1]) << ':' << dye::purple(argv[2]) << dye::green(" Ping: ") << dye::light_purple(ping) << dye::purple("ms ") << "\n";
			success = true;
		} else {
			std::cout << dye::red("Connection timed out reached (2 sec)\n");
		}
		connected = true;
		close_net(conn);
		if (success) {std::this_thread::sleep_for(std::chrono::milliseconds(1000));}
		success = false;
	}
}