#include "stdafx.h"
#include <conio.h>
#include <time.h>
#include <iostream>

/*
Simple udp client
Silver Moon (m00n.silv3r@gmail.com)
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define SERVER "127.0.0.1"  //ip address of udp server
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

int main(void)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	//start communication

	int point = 0;
	memset(message, '\0', BUFLEN);
	while (1)
	{
		printf("Enter message : ");
		gets_s(message);

		//send the message
		/*
		if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}*/
		point = 0;
		
		while (_kbhit()){
			char cur = _getch();
			if (cur == 13){
				if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
				{
					printf("sendto() failed with error code : %d", WSAGetLastError());
					exit(EXIT_FAILURE);
				}
				memset(message, '\0', BUFLEN);
				point = 0;
			}
			else
			{
				if (point > BUFLEN - 2)
					point = BUFLEN - 2;
				message[point] = cur;
				point++;
			}
		}
		
		//receive a reply and print it
		//clear the buffer by filling null, it might have previously received data

		FD_SET fds;
		struct timeval tv;

		FD_ZERO(&fds);
		FD_SET(s, &fds);

		tv.tv_sec = 0;
		tv.tv_usec = 30000;

		int n = select(s, &fds, NULL, NULL, &tv);
		memset(buf, '\0', BUFLEN);
		
		//try to receive some data, this is a blocking call
		if (n > 0){
			memset(buf, '\0', BUFLEN);
			if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR){
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			puts(buf);
			printf("\r",buf,"\n");
		}
		else if (n == 0) {
			
		}
		else if (n < 0){
			printf("Error");
			exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}