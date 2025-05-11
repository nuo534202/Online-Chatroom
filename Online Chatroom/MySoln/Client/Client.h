// Client.h
// include all declaration of struct and subfunctions in Client.cpp
#pragma once

#include <winsock2.h>
#include <string>
using namespace std;

/*
*	function: get message from server socket
*	parameter:
*		SOCKET sock: the server socket
*	return type: int
*/
int recvThread(SOCKET sock);