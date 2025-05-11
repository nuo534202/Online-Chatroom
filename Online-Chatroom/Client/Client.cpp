// Client.cpp 

#define _WINSOCK_DEPRECATED_NO_WARNINGS   // Disable deprecated API warnings
#define _CRT_SECURE_NO_WARNINGS           // Disable secure function warnings

#include <iostream>
#include <string>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")   // Link the library of "ws2_32.lib" 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <easyx.h>
#include <graphics.h>
#include <queue>
#include "Client.h"

#define DEFAULT_PORT 50000

LPCTSTR msg_q[10];
int ft = 0, bk = -1, sz = 0;

int main(int argc, char** argv) {

	char szBuff[100];
	int msg_len;
	//int addr_len;
	struct sockaddr_in server_addr;
	struct hostent* hp;
	SOCKET connect_sock;
	WSADATA wsaData;

	const char* server_name = "localhost";
	unsigned short port = DEFAULT_PORT;
	unsigned int addr;

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
	{
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		//WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}

	if (isalpha(server_name[0]))
		hp = gethostbyname(server_name);
	else
	{
		addr = inet_addr(server_name);
		hp = gethostbyaddr((char*)&addr, 4, AF_INET);
	}

	if (hp == NULL)
	{
		fprintf(stderr, "Cannot resolve address: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//copy the resolved information into the sockaddr_in structure
	memset(&server_addr, 0, sizeof(server_addr));
	memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
	server_addr.sin_family = hp->h_addrtype;
	server_addr.sin_port = htons(port);

	connect_sock = socket(AF_INET, SOCK_STREAM, 0);	// TCP socket

	if (connect_sock == INVALID_SOCKET) {
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Client connecting to: %s\n", hp->h_name);

	if (connect(connect_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		fprintf(stderr, "connect() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	string username;
	while (1) {
		printf("Input your username (cannot be modified): ");
		getline(cin, username);
		while (username.size() && (username.back() == '\n' || username.back() == ' '))
			username.pop_back();
		if (username != "") break;
	}
	msg_len = send(connect_sock, username.c_str(), (int)username.size(), 0);

	thread t(recvThread, connect_sock);
	t.detach();

	while (1) {
		printf("input a command string: ");

		if (fgets(szBuff, sizeof(szBuff), stdin) == NULL)
			break; // EOF or error

		szBuff[strcspn(szBuff, "\n")] = '\0';
		msg_len = send(connect_sock, szBuff, strlen(szBuff), 0);

		if (msg_len == SOCKET_ERROR) {
			fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		if (msg_len == 0) {
			printf("server closed connection\n");
			closesocket(connect_sock);
			WSACleanup();
			return -1;
		}
	}

	closesocket(connect_sock);
	WSACleanup();
}

int recvThread(SOCKET sock) {
	/*initWindow();*/
	char buf[1024];
	while (1) {
		int len = recv(sock, buf, sizeof(buf) - 1, 0);
		if (len <= 0) break;
		if (len == SOCKET_ERROR) {
			fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
			closesocket(sock);
			WSACleanup();
			return -1;
		}

		if (len == 0) {
			printf("server closed connection\n");
			closesocket(sock);
			WSACleanup();
			return -1;
		}
		buf[len] = '\0';

		/*showMessage(string(buf));*/
		printf("\nReceive Message: %s\ninput a command string: ", buf);
	}
}