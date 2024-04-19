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
#include <iostream>
#include <chrono>
#include <thread>

void connect_net(const char* ip, const char* port, int* connRet, const unsigned short int protocol) { // 0 for TCP | 1 for UDP
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int conn = -1;
	if (protocol == 0) {conn = socket(AF_INET, SOCK_STREAM, 0);}
	else if (protocol == 1) {conn = socket(AF_INET, SOCK_DGRAM, 0);}
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	if (connect(conn, (struct sockaddr*)&addr, sizeof(addr)) < 0) {conn = -1;}
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
	while (!timedOut && passed<timeoutMS && conn < 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		passed = passed+10;
	}
	return conn;
}

std::string setcolor(std::string input, int color=0) {
	// WHITE 0
	// GREEN 1
	// RED   2
	// PINK  3
	// GRAY  4
	#ifdef __WIN32
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		if (color == 0) {SetConsoleTextAttribute(console, 7);}
		else if (color == 1) {SetConsoleTextAttribute(console, 10);}
		else if (color == 2) {SetConsoleTextAttribute(console, 4);}
		else if (color == 3) {SetConsoleTextAttribute(console, 13);}
		else if (color == 4) {SetConsoleTextAttribute(console, 8);}
		return input;
	#else
		std::string colorCode = "";
		if (color == 0) {colorCode = "\x1B[37m";}
		else if (color == 1) {colorCode = "\x1B[32m";}
		else if (color == 2) {colorCode = "\x1B[31m";}
		else if (color == 3) {colorCode = "\x1B[35m";}
		else if (color == 4) {colorCode = "\x1B[97m";}
		return colorCode + input;
	#endif
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Usage: statping IP PORT\n";
		return -1;
	}
	while (true) {
		int ping = 0;
		bool connected = false;
		bool success = false;
		std::thread timerThread(timer, &ping, &connected);
		timerThread.detach();
		int conn = connectWithTimeOut(2000, argv[1], argv[2]);
		if (conn > 0) {
			std::cout << setcolor("Connected ", 1) << setcolor(argv[1], 4) << ':' << argv[2] << setcolor(" Ping: ", 1) << setcolor(std::to_string(ping)) << setcolor("ms ") << "\n";
			success = true;
		} else {
			std::cout << setcolor("Connection timed out reached", 2) << setcolor("\n");
		}
		connected = true;
		close_net(conn);
		if (success) {std::this_thread::sleep_for(std::chrono::milliseconds(1500));}
		success = false;
	}
}