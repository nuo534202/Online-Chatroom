// Server.cpp 

#define _WINSOCK_DEPRECATED_NO_WARNINGS   // Disable deprecated API warnings
#define _CRT_SECURE_NO_WARNINGS           // Disable secure function warnings

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")   // Link the library of "ws2_32.lib" 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <thread>
#include <WS2tcpip.h>
#include <iostream>
#include "Server.h"
using namespace std;

#define DEFAULT_PORT 50000

vector<User> users; // users information
int groupId = 0; // next group id
mutex usersMutex; // users' mutex

int main(int argc, char** argv) {

	int addr_len;
	sockaddr_in local, client_addr;
	SOCKET sock, msg_sock;
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		// WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}

	// Fill in the address structure
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(DEFAULT_PORT);

	sock = socket(AF_INET, SOCK_STREAM, 0);	// TCP socket

	if (sock == INVALID_SOCKET) {
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	BOOL reuse = TRUE;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
		fprintf(stderr, "setsockopt(SO_REUSEADDR) failed with error %d\n", WSAGetLastError());
	}

	if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
		fprintf(stderr, "bind() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// waiting for connections
	if (listen(sock, 5) == SOCKET_ERROR) {
		fprintf(stderr, "listen() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Waiting for connections ........\n");
	while (1) {
		addr_len = sizeof(client_addr);
		msg_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
		if (msg_sock == INVALID_SOCKET) {
			fprintf(stderr, "accept() failed with error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		char username[100];
		int n = recv(msg_sock, username, sizeof(username) - 1, 0);
		if (n <= 0) {
			closesocket(sock);
			continue;
		}
		username[n] = '\0';

		users.push_back(User(msg_sock, string(username)));

		printf("accepted connection from %s, port %d, username %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), username);

		thread t(handleClient, client_addr, msg_sock);
		t.detach();
	}

	return 0;
}

void RemoveUserBySock(SOCKET s) {
	auto it = remove_if(users.begin(), users.end(), [&](const User& u) {
		return u.sock == s;
	});
	users.erase(it, users.end());
}

User* FindUserByName(const string& name) {
	for (int i = 0; i < (int)users.size(); i++)
		if (users[i].name == name) return &users[i];
	return nullptr;
}

User* FindUserBySock(SOCKET s) {
	for (int i = 0; i < (int)users.size(); i++)
		if (users[i].sock == s) return &users[i];
	return nullptr;
}

void BroadcastToUser(const User& u, const string& msg) {
	send(u.sock, msg.c_str(), (int)msg.size(), 0);
}

void BroadcastGroup(int gid, const string& msg) {
	for (int i = 0; i < (int)users.size(); i++) {
		if (users[i].groups.count(gid)) {
			BroadcastToUser(users[i], msg);
		}
	}
}

void CreateGroup(vector<User>& v) {
	groupId++;
	for (const User& u : v) {
		for (int i = 0; i < users.size(); i++) {
			if (users[i] == u) {
				users[i].groups.insert(groupId);
			}
		}
	}
}

void addUser(int gid, const string &name) {
    for (int i = 0; i < users.size(); i++)
        if (users[i].name == name) {
            users[i].groups.insert(gid);
            break;
        }
}

void deleteUser(int gid, const string &name) {
    for (int i = 0; i < users.size(); i++)
        if (users[i].name == name) {
            users[i].groups.erase(gid);
            break;
        }
}

void eraseFrontSpace(string& msg) {
	int pos = 0;
	while (pos + 1 < (int)msg.size() && msg[pos] == ' ') pos++;
	msg.erase(0, pos);
}

string HandleCommand(SOCKET s, const string& buf) {
	istringstream iss(buf);
	string cmd;
	iss >> cmd;

	// new username1, username2, username3, ...
	if (cmd == "new") {
		vector<User> v;
		string uname;
		while (iss >> uname)
			for (const User &u : users)
				if (u.name == uname)
					v.push_back(u);
		if (v.size() == 0) return "Invalid User Input!";
		CreateGroup(v);
		string out = "Group is created successfully with group id " + to_string(groupId);
		send(s, out.c_str(), (int)out.size(), 0);
		return "";
	}

	// add/delete gid username
	if (cmd == "add" || cmd == "delete") {
		int gid;
		string uname;
		if (!(iss >> gid)) {
			return "Invalid Group Id Input!";
		}
		if (!(iss >> uname)) {
			return "Invalid Username Input!";
		}
		User* u = FindUserByName(uname);
		if (!u) {
			string out = "ERROR: no such user\n";
			send(s, out.c_str(), (int)out.size(), 0);
		}
		if (cmd == "add") {
			addUser(gid, u->name);
			string out = "add " + u->name + " to group " + to_string(gid) + " successfully";
			send(s, out.c_str(), (int)out.size(), 0);
		}
		else {
			deleteUser(gid, u->name);
			string out = "delete " + u->name + " from group " + to_string(gid) + " successfully";
			send(s, out.c_str(), (int)out.size(), 0);
		}
		return "";
	}

	// all/group/user gid/username msg
	string msg;
	if (cmd == "all") {
		if (!(getline(iss, msg))) return "No Message!";
		eraseFrontSpace(msg);
		for (int i = 0; i < users.size(); i++)
			BroadcastToUser(users[i], msg);
		return "";
	}
	else if(cmd == "group") {
		int gid;
		if (!(iss >> gid)) {
			return "Invalid Group Id Input!";
		}
		if (!(getline(iss, msg))) {
			return "No Message!";
		}
		eraseFrontSpace(msg);
		BroadcastGroup(gid, msg);
		return "";
	}
	else if (cmd == "user") {
		string username;
		if (!(iss >> username)) {
			return "Invalid Username Input!";
		}
		if (!(getline(iss, msg))) {
			return "No Message!";
		}
		eraseFrontSpace(msg);
		for (int i = 0; i < users.size(); i++) {
			if (users[i].name == username) {
				BroadcastToUser(users[i], msg);
			}
		}
		return "";
	}
	return "Invalid Command Input!";
}

int handleClient(sockaddr_in client_addr, SOCKET msg_sock) {
	int msg_len;
	char szBuff[100];
	while (1) {
		msg_len = recv(msg_sock, szBuff, sizeof(szBuff) - 1, 0);

		if (msg_len == SOCKET_ERROR) {
			fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		if (msg_len == 0) {
			printf("Client closed connection\n");
			closesocket(msg_sock);
			return -1;
		}
		szBuff[msg_len] = '\0';
		printf("Bytes Received: %d from %s, command: %s\n", msg_len, inet_ntoa(client_addr.sin_addr), szBuff);
		string fb = HandleCommand(msg_sock, string(szBuff));
		if (fb != "") send(msg_sock, fb.c_str(), fb.size(), 0);

		//msg_len = send(msg_sock, szBuff, msg_len, 0);
		if (msg_len == 0) {
			printf("Client closed connection\n");
			closesocket(msg_sock);
			return -1;
		}
	}
	closesocket(msg_sock);
	WSACleanup();
}