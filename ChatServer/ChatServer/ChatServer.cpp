#include "stdafx.h"
#include <iostream>
#include <list>
#include <vector>
/*
Simple UDP Server
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data
#define UNLEN 25 //Max length of user name 

using namespace std;

string serverAnswer = "none";
vector<struct sockaddr_in> generalusers;

struct User {
	string _name;
	sockaddr_in _userip;
	sockaddr_in _userport;
	string room;
};

list<User> userList;

void SetUserName(sockaddr_in loginip, string newname) {
	User k;
	k._name = newname;
	k._userip.sin_addr = loginip.sin_addr;
	k._userport.sin_port = loginip.sin_port;
	userList.push_back(k);
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
	for (size_t i = 0; i < userList.size(); i++){
		if (it->_userip.sin_addr.S_un.S_addr == checkip.sin_addr.S_un.S_addr && it->_userport.sin_port == checkip.sin_port){
			exists = true;
			break;
		}
		it++;
	}
	return exists;
}

void ServerReturn(){

}

void Command(string _buf, sockaddr_in user) {
	char auxstring[UNLEN];
	int auxnum = 0;
	memset(auxstring, '\0', UNLEN);
	if (_buf == "#//<1/0>") {
		printf("SHUTTING DOWN SERVER.\n");
		exit(EXIT_FAILURE);
	}
	if (_buf == "#chinesemafia") {
		printf("(-_(-_(-_-)_-)_-)\n");
		serverAnswer = "(-_(-_(-_-)_-)_-)\n";
	}
	if (_buf[1] == 'l' && _buf[2] == 'o' && _buf[3] == 'g' && _buf[4] == 'i' && _buf[5] == 'n' && _buf[6] == ' ') {
		printf("Logging new user\n");
		if (!CheckUserIdentity(user)){
			if (_buf.length() - 7 > UNLEN - 1){
				serverAnswer = "Username too long. Max characters is 24";
			}
			else{
				for (size_t i = 7; i < _buf.length(); i++) {
					auxstring[auxnum] = _buf[i];
					auxnum++;
				}
				SetUserName(user, auxstring);
				generalusers.push_back(user);
				serverAnswer = "Welcome user " + GUserName(user);
			}
		}
		else{
			serverAnswer = "You are already logged in!\n";
		}
	}
	else{
		printf("Data: %s\n", "Invalid Command.");
	}
}

int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR){
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	//keep listening for data
	//list<struct sockaddr_in>::iterator itlogged = generalusers.begin();
	while (1){
		printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR){
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		
		if (CheckUserIdentity(si_other)){
			if (buf[0] == '#') {
				Command(buf, si_other);
			}
			else{
				cout<<GUserName(si_other).c_str()<<": "<<buf<<endl;
				serverAnswer = GUserName(si_other) + ": "+ buf;
			}
		}
		else{
			if (buf[0] == '#') {
				Command(buf, si_other);
			}
			else{
				printf("User not registered, use #login \n");
				serverAnswer = "User not registered, use #login + your desired name";
			}
		}

		//now reply the client with the same data
		/*
		if (sendto(s, serverAnswer.c_str(), serverAnswer.length(), 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		*/
		//itlogged = generalusers.begin();
		for (size_t i = 0; i < generalusers.size(); i++){
			if (sendto(s, serverAnswer.c_str(), serverAnswer.length(), 0, (struct sockaddr*) &generalusers[i], slen) == SOCKET_ERROR){
				printf("sendto() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			else{
				//itlogged++;
			}
		}
		//itlogged = generalusers.begin();
	}

	closesocket(s);
	WSACleanup();

	return 0;
}