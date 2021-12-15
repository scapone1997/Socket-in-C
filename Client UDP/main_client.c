/*
 * main_client.c
 *
 *  Created on: 14 dic 2021
 *      Author: simoc
 */

#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h> /* for memset() */
#include "protocol.h"

//Asking user to insert a string
int insert_string(char* str, int length){
	int number = 1;
	while(number == 1){
		printf("Enter the operation in the following format [Operation Integer Integer] (for example + 24 45): \n");
		fgets(str, length, stdin);
		fflush(stdin);

		int count = 0;
		if(str[0] == '+' || str[0] == '-' || str[0] == 'x' || str[0] == '/'){
			for(int k = 0; str[k] != '\0'; k++){
				if(str[k] ==  ' ')
					count++;
			}
			if(count == 2)
				number = 0;
		}
		else if(str[0] == '='){
			number = 2;
		}
		if(number == 1)
			printf("Wrong format. Please try again.\n");
	}
	return number;
}

void ErrorHandler(char *errorMessage) {
	printf(errorMessage);
}

void ClearWinSock() {
	#if defined WIN32
	WSACleanup();
	#endif
}

int main(int argc, char* argv[]) {
	#if defined WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2 ,2), &wsaData);
	if (iResult != 0) {
		printf ("error at WSASturtup\n");
		return EXIT_FAILURE;
	}
	#endif

	int sock;
	struct sockaddr_in echoServAddr;//sad
	struct sockaddr_in fromAddr;
	unsigned int fromSize;
	char echoBuffer[ECHOMAX];
	int echoStringLen;
	int respStringLen;
	char serverAddress[25];
	char serverName[25];
	int serverPort;
	struct hostent *serverHost;

	//If the argv vector contains parameters then we do not use the default values..
	if(argc > 1){
		char delim[] = ":";
		char *ptr = strtok(argv[1], delim);
		char uno[25];
		char due[25];

		strcpy(uno, ptr);
		ptr = strtok(NULL, delim);

		strcpy(due, ptr);
		ptr = strtok(NULL, delim);

		serverPort = atoi(due);
		strcpy(serverName, uno);
		serverHost = gethostbyname(serverName);
		if (serverHost == NULL) {
			ErrorHandler("gethostbyname() client failed.\n");
			return -1;
		} else {
			struct in_addr *hostAddr = (struct in_addr*) serverHost->h_addr_list[0];
			strcpy(serverAddress, inet_ntoa(*hostAddr));
		}
	}
	else
	{
		//otherwise we use the default values
		strcpy(serverAddress, IP);
		strcpy(serverName, NAME);
		serverPort = PORT;
	}

	//Creating the socket
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		ErrorHandler("socket client creation failed.\n");
		closesocket(sock);
		ClearWinSock();
		system("pause");
		return -1;
	}

	//Creating the Server address
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = PF_INET;
	echoServAddr.sin_port = htons(serverPort);
	echoServAddr.sin_addr.s_addr = inet_addr(serverAddress);

	while(1){
		char string[25];
		int stop = insert_string(string, 25);
		if(stop == 2)
			break;

		if ((echoStringLen = strlen(string)) > ECHOMAX)
			ErrorHandler("echo word too long");

		//Sending the message to Server
		if (sendto(sock, string, echoStringLen, 0, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) != echoStringLen){
			ErrorHandler("sendto() sent different number of bytes than expected");
			closesocket(sock);
			ClearWinSock();
			system("pause");
			return -1;
		}


		//Reading the response received from the server
		fromSize = sizeof(fromAddr);
		respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr*)&fromAddr, &fromSize);
		echoBuffer[respStringLen] = '\0';
		if (respStringLen <= 0) {
			ErrorHandler("recvfrom newString failed.\n");
			closesocket(sock);
			ClearWinSock();
			system("pause");
			return -1;
		}

		//Retrive canonic server name
		/*
		struct hostent* serverIP;
		serverIP = gethostbyaddr((char*) &echoServAddr.sin_addr.s_addr, 4, AF_INET);
		struct in_addr* IP_A = (struct in_addr*) serverIP->h_addr_list[0];
		*/

		if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
			fprintf(stderr, "Error: received a packet from unknown source.\n");
			closesocket(sock);
			ClearWinSock();
			exit(EXIT_FAILURE);
		}
		printf("Result from server %s, ip %s: %s\n\n", serverName, serverAddress, echoBuffer);
	}

	closesocket(sock);
	ClearWinSock();
	system("pause");
	return 0;
}
