// Server.h
// include all declaration of structs and subfunctions in Server.cpp
#pragma once

#include <vector>
#include <mutex>
#include <string>
#include <winsock2.h>
#include <unordered_map>
#include <set>
using namespace std;

// a users' type
struct User {
	SOCKET sock; // users' socket
	string name; // username
	set<int> groups; // which group the user is in
	User(SOCKET _sock, string _name) {
		sock = _sock;
		name = _name;
	}
	bool operator== (const User& u) const {
		return sock == u.sock && name == u.name && groups == u.groups;
	}
};

/*
*	function: remove user by socket s
*	parameter:
		SOCKET s: socket s of user to be deleted
*	return type: void
*/
void RemoveUserBySock(SOCKET s);

/*
*	function: use user's name to find the corresponding User struct
*	parameter:
*		string &name: reference to users' name
*	return type: User* (User-type pointer)
*/
User* FindUserByName(const string& name);

/*
*	function: use a socket to find user
*	parameter:
*		SOCKET s: a users' socket
*	return type: User* (User-type pointer)
*/
User* FindUserBySock(SOCKET s);

/*
*	function: Broadcast to a single user
*	parameter:
*		User &u: reference of User u
*		string &msg: reference of message to be sent
*	return type: void
*/
void BroadcastToUser(const User &u, const string &msg);

/*
*	function: Broadcast to a group
*	parameter:
*		int gid: group id
*		string &msg: reference of message to be sent
*	return type: void
*/
void BroadcastGroup(int gid, const string &msg);

/*
*	function: create a new group
*	parameter:
*		vector<User> &v: all the users need to be created in a new group
*	function: void
*/
void CreateGroup(vector<User>& v);

/*
*	function: add user u to group gid
*	parameter:
*		int gid: group id
*		string &name: reference to username
*	return type: void
*/
void addUser(int gid, const string &name);

/*
*	function: delete user u in group gid
*	parameter:
*		int gid: group id
*		string &name: reference to username
*	return type: void
*/
void deleteUser(int gid, const string &name);

/*
*	function: process client request. use buf to get corresponding action
*	parameter:
*		SOCKET s:
*		string &buf: message from user
*	return type: void
*/
string HandleCommand(SOCKET s, const string &buf);

/*
*	function: handle client with socket s
*	parameter:
*		sockaddr_in client_addr: a struct store information of client address
*		SOCKET msg_sock: clients' socket
*	return type: int
*/
int handleClient(sockaddr_in client_addr, SOCKET msg_sock);

/*
*	function: erase the space at the front of the string
*	parameter:
*		string &msg: input string
*	return type: void
*/
void eraseFrontSpace(string& msg);