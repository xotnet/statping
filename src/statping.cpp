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

void connect_net(const char* ip, const char* port, int* conn, const unsigned short int protocol) { // 0 for TCP | 1 for UDP
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int sock = 0;
	if (protocol == 0) {sock = socket(AF_INET, SOCK_STREAM, 0);}
	else if (protocol == 1) {sock = socket(AF_INET, SOCK_DGRAM, 0);}
	if (sock < 0) {*conn = -3; return;}
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {*conn = -4; return;}
	*conn = sock;
}

int close_net(int conn) {
	#ifdef __WIN32
		return closesocket(conn);
	#elif __linux__
		return close(conn);
	#endif
}

void timer(int* ping, int* conn) {
	while (*conn < 1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		*ping = *ping + 1;
	}
}

void timeOutCheck(int howLongMS, bool* timedOut) {
	std::this_thread::sleep_for(std::chrono::milliseconds(howLongMS));
	*timedOut = true;
}

int connectWithTimeOut(int timeoutMS, char* ip, char* port) {
	bool timedOut = false;
	int conn = -2;
	std::thread timeOutCheckThread(timeOutCheck, timeoutMS, &timedOut);
	timeOutCheckThread.detach();
	std::thread connectThread(connect_net, ip, port, &conn, 0);
	connectThread.detach();
	while (!timedOut && conn < 1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

std::string resolve_net(const char* domain, const char* port) {
	#ifdef __WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	#endif
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);
	getaddrinfo(domain, port, &hints, &res);
	struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
	char ipstr[INET_ADDRSTRLEN];
	inet_ntop(res->ai_family, &addr->sin_addr, ipstr, sizeof(ipstr));
	return ipstr;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << setcolor("Usage: statping IP PORT", 2) << setcolor("\n");
		return -1;
	}
	std::string address = argv[1];
	address = resolve_net(&address[0], argv[2]);
	std::cout << setcolor("Resolved ", 4) << address << setcolor("\n");
	bool slash = false;
	while (true) {
		char slashchar = '/';
		if (slash == true) {slashchar = '\\'; slash = false;}
		else {slashchar = '/'; slash = true;}
		int ping = 0;
		int conn = 0;
		std::thread timerThread(timer, &ping, &conn);
		timerThread.detach();
		conn = connectWithTimeOut(3000, &address[0], argv[2]);
		if (conn > 0) {
			std::cout << setcolor("[", 1) << slashchar << "] Connected " << setcolor(&address[0], 4) << ':' << argv[2] << setcolor(" Ping: ", 1) << setcolor(std::to_string(ping)) << setcolor("ms ") << "\n";
		} else {
			std::cout << setcolor("[", 2) << slashchar << "] Connection timed out reached" << setcolor("\n");
		}
		close_net(conn);
		if (conn > 0) {std::this_thread::sleep_for(std::chrono::milliseconds(1500));}
	}
}