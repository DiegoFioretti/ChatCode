#include "stdafx.h"
#include <iostream>
#include <list>
#include <vector>
//#include <windows.h>
/*
Simple UDP Server
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data
#define UNLEN 25	//Max length of user name 
#define ROOMLEN 25	//Max length of room name

using namespace std;

char serverAnswer [BUFLEN];
vector<struct sockaddr_in> generalusers;
vector<string> rooms;

struct User {
	string _name;
	sockaddr_in _userip;
	sockaddr_in _userport;
	string room;
};

list<User> userList;

void WriteServerAnswer(string answer) {
	for (size_t i = 0; i < answer.length(); i++){
		serverAnswer[i] = answer[i];
	}
}

void SetUserName(sockaddr_in loginip, string newname) {
	User k;
	k._name = newname;
	k._userip.sin_addr = loginip.sin_addr;
	k._userport.sin_port = loginip.sin_port;
	k.room = rooms[0];
	userList.push_back(k);
}

bool CheckExistingRooms(string roomname) {
	for (size_t i = 0; i < rooms.size(); i++){
		if (rooms[i] == roomname){
			return true;
		}
	}
	return false;
}

string GUserRoom(sockaddr_in user) {
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
			return it->room;
		}
		it++;
	}
	return "none";
}

void MoveUserRoom(sockaddr_in user, string room) {
	int aux = 0;
	for (size_t i = 0; i < rooms.size(); i++) {
		if (rooms[i] == room) {
			aux = i;
		}
	}
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == user.sin_addr.S_un.S_addr && it->_userport.sin_port == user.sin_port) {
			it->room = rooms[aux];
		}
		it++;
	}
}

string GUserName(sockaddr_in userip) {
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == userip.sin_addr.S_un.S_addr && it->_userport.sin_port == userip.sin_port) {
			return it->_name.c_str();
		}
		it++;
	}
	return "none";
}

bool CheckUserIdentity(sockaddr_in checkip) {
	bool exists = false;
	list<User>::iterator it = userList.begin();
	for (size_t i = 0; i < userList.size(); i++) {
		if (it->_userip.sin_addr.S_un.S_addr == checkip.sin_addr.S_un.S_addr && it->_userport.sin_port == checkip.sin_port) {
			exists = true;
			break;
		}
		it++;
	}
	return exists;
}

void Command(string _buf, sockaddr_in user) {
	char auxstring[UNLEN];
	int auxnum = 0;
	memset(auxstring, '\0', UNLEN);
	//#//<1/0> Shuts downs the server
	if (_buf == "#//<1/0>") {
		printf("SHUTTING DOWN SERVER.\n");
		exit(EXIT_FAILURE);
	}
	//#login Creates a new user
	else if (_buf[1] == 'l' && _buf[2] == 'o' && _buf[3] == 'g' && _buf[4] == 'i' && _buf[5] == 'n' && _buf[6] == ' ') {
		printf("Logging new user\n");
		if (!CheckUserIdentity(user)) {
			if (_buf.length() - 7 > UNLEN - 1) {
				WriteServerAnswer("Username too long. Max characters is " + UNLEN + '\n');
			}
			else {
				for (size_t i = 7; i < _buf.length(); i++) {
					auxstring[auxnum] = _buf[i];
					auxnum++;
				}
				SetUserName(user, auxstring);
				generalusers.push_back(user);
				WriteServerAnswer("Welcome user " + GUserName(user) + '\n');
			}
		}
		else {
			printf("Unlogged user tried to create room\n");
		}
	}
	//#createroom Creates a new room
	else if (_buf[1] == 'c' && _buf[2] == 'r' && _buf[3] == 'e' && _buf[4] == 'a' && _buf[5] == 't' && _buf[6] == 'e' && _buf[7] == 'r' && _buf[8] == 'o' && _buf[9] == 'o' && _buf[10] == 'm' &&_buf[11] == ' '){
		printf("Creating new room\n");
		if (CheckUserIdentity(user)) {
			if (_buf.length() - 7 > ROOMLEN - 1) {
				WriteServerAnswer("Name of the room too long. Max characters is " + ROOMLEN + '\n');
			}
			else {
				for (size_t i = 12; i < _buf.length(); i++) {
					auxstring[auxnum] = _buf[i];
					auxnum++;
				}
				if (CheckExistingRooms(auxstring)) {
					printf("User tried to create a room with a same name to an existing one\n");
					WriteServerAnswer("Name for room already taken");
				}
				else {
					rooms.push_back(auxstring);
					printf("Created new room %s\n", auxstring);
					WriteServerAnswer("Room was created succesfully\n");
				}
			}
		}
	}
	//#checkrooms Check aexisting and current rooms
	else if (_buf[1] == 'c' && _buf[2] == 'h' && _buf[3] == 'e' && _buf[4] == 'c' && _buf[5] == 'k' && _buf[6] == 'r' && _buf[7] == 'o' && _buf[8] == 'o' && _buf[9] == 'm' &&_buf[10] == 's') {
		if (CheckUserIdentity(user)) {
				printf("User %s is checking their room\n", GUserName(user));
				WriteServerAnswer("You are currently in the " + GUserRoom(user) + " room");
			}
	}
	//#movetoroom Moves to another room
	else if (_buf[1] == 'm' && _buf[2] == 'o' && _buf[3] == 'v' && _buf[4] == 'e' && _buf[5] == 't' && _buf[6] == 'o' && _buf[7] == 'r' && _buf[8] == 'o' && _buf[9] == 'o' && _buf[10] == 'm' &&_buf[11] == ' ') {
		if (CheckUserIdentity(user)) {
			for (size_t i = 12; i < _buf.length(); i++) {
				auxstring[auxnum] = _buf[i];
				auxnum++;
			}
			if (!CheckExistingRooms(auxstring)) {
				printf("User tried to join a room the doesnt exist\n");
				WriteServerAnswer("The room you want to join doesnt exist");
			}
			else {
				printf("Moving %s to another room\n", GUserName(user));
				MoveUserRoom(user, auxstring);
				WriteServerAnswer("User " + GUserName(user) + " was moved to room " + auxstring);
			}
		}
	}
	else {
		printf("Data: Invalid Command\n");
		WriteServerAnswer("Sorry, the command you used doesnt exist\n");
	}
}

int main(){
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	rooms.push_back("general");

	//keep listening for data
	while (1) {

		fflush(stdout);
		
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		memset(serverAnswer, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		FD_SET fds;
		struct timeval tv;

		FD_ZERO(&fds);
		FD_SET(s, &fds);

		tv.tv_sec = 0;
		tv.tv_usec = 30000;

		int n = select(s, &fds, NULL, NULL, &tv);
		if (n>0) {
			if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR) {
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}

			//print details of the client/peer and the data received
			printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));

			if (CheckUserIdentity(si_other)) {
				if (buf[0] == '#') {
					Command(buf, si_other);
				}
				else {
					cout << GUserName(si_other).c_str() << ": " << buf << endl;
					WriteServerAnswer(GUserName(si_other) + ": " + buf);
				}
			}
			else {
				if (buf[0] == '#' && buf[1] == 'l' && buf[2] == 'o' && buf[3] == 'g' && buf[4] == 'i' && buf[5] == 'n' && buf[6] == ' ') {
					Command(buf, si_other);
				}
				else {
					printf("User not registered, use #login \n");
					WriteServerAnswer("User not registered, use #login + your desired name\n");
					if (sendto(s, serverAnswer, BUFLEN, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR) {
						printf("sendto() failed with error code : %d", WSAGetLastError());
						exit(EXIT_FAILURE);
					}
				}
			}
			//now reply the client with the same data
			for (size_t i = 0; i < generalusers.size(); i++) {
				if (GUserRoom(si_other) == GUserRoom(generalusers[i])){
					if (sendto(s, serverAnswer, BUFLEN, 0, (struct sockaddr*) &generalusers[i], slen) == SOCKET_ERROR) {
						printf("sendto() failed with error code : %d", WSAGetLastError());
						exit(EXIT_FAILURE);
					}
				}
			}
			cout << GUserRoom(si_other).c_str() << " | " << GUserName(si_other).c_str() << " : " << serverAnswer << endl;
			//printf("\n %s", serverAnswer);
		}
		else if (n == 0) {
			printf("Waiting for data...\r");
		}
		else if (n<0) {
			printf("error");
		}

		fflush(stdin);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}