/*
 * main_server.c
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

struct message{
	char operation;
	int first;
	int second;
	float result;
	int error;
};

//Converting a string into a message for Server
void convert_message(char* str, struct message* mess){
	char delim[] = " ";
	char *ptr = strtok(str, delim);
	char uno[25];
	char due[25];
	char tre[25];

	strcpy(uno, ptr);
	ptr = strtok(NULL, delim);

	strcpy(due, ptr);
	ptr = strtok(NULL, delim);

	strcpy(tre, ptr);
	ptr = strtok(NULL, delim);

	mess->operation = uno[0];
	mess->first = atoi(due);
	mess->second = atoi(tre);
	mess->result = 0;
	mess->error = 0;
}

int mult(int first, int second){
	return (first*second);
}

int add(int first, int second){
	return (first + second);
}

int sub(int first, int second){
	return (first - second);
}

float division(int first, int second){
	return ((float)first/(float)second);
}


//Function that performs the operation requested by the user
void operation(struct message* mess){
	if(mess->operation == '+'){
		mess->result = add(mess->first, mess->second);
	}
	if(mess->operation == '-') {
		mess->result = sub(mess->first, mess->second);
	}
	if(mess->operation == 'x') {
		mess->result = mult(mess->first, mess->second);
	}

	if(mess->operation == '/') {
		if(mess->second == 0)
		{
			mess->result = 0;
			mess->error = 1;
		}
		else  mess->result = division(mess->first, mess->second);
	}
}

void result_to_string(char* stringRcvd, struct message* mess){
	char result[25];
	char uno[25];
	char due[25];
	char oper[25];
	char finalString[50];

	sprintf(uno, "%d", mess->first);
	sprintf(due, "%d", mess->second);
	oper[0] = mess->operation;
	oper[1] = '\0';
	sprintf(result, "%g", mess->result);

	finalString[0] = '\0';
	strcat(finalString, uno);
	strcat(finalString, " ");
	strcat(finalString, oper);
	strcat(finalString, " ");
	strcat(finalString, due);
	strcat(finalString, " = ");

	if(mess->error == 1)
		strcat(finalString, "error");
	else strcat(finalString, result);

	strcpy(stringRcvd, finalString);
}

void ErrorHandler(char *errorMessage) {
	printf(errorMessage);
}

void ClearWinSock() {
	#if defined WIN32
	WSACleanup();
	#endif
}

int main() {
	#if defined WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2 ,2), &wsaData);
	if (iResult != 0) {
		printf ("error at WSASturtup\n");
		return EXIT_FAILURE;
	}
	#endif

	int sock;
	struct sockaddr_in echoServAddr;
	struct sockaddr_in echoClntAddr;
	unsigned int cliAddrLen;
	char echoBuffer[ECHOMAX];
	int recvMsgSize;

	//Creating the socket
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		ErrorHandler("socket() failed");
		closesocket(sock);
		ClearWinSock();
		system("pause");
		return -1;
	}

	//Creating the Server Address
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_port = htons(PORT);
	echoServAddr.sin_addr.s_addr = inet_addr(IP);

	//Binding of socket
	if ((bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))) < 0){
		ErrorHandler("bind() failed");
		closesocket(sock);
		ClearWinSock();
		system("pause");
		return -1;
	}

	printf("UDP Server is available...\n");
	//Receiving the string from Client
	while(1) {
		cliAddrLen = sizeof(echoClntAddr);
		recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr*)&echoClntAddr, &cliAddrLen);
		if (recvMsgSize <= 0) {
			ErrorHandler("recvfrom server failed.\n");
			closesocket(sock);
			ClearWinSock();
			system("pause");
			return -1;
		}
		echoBuffer[recvMsgSize-1] = '\0';

		//Retrive client information and print his request
		struct hostent* clientIP;
		clientIP = gethostbyaddr((char*) &echoClntAddr.sin_addr.s_addr, 4, AF_INET); //retrive canonic client name
		struct in_addr* IP_A = (struct in_addr*) clientIP->h_addr_list[0];
		printf("Request operation '%s' from client %s, ip %s\n", echoBuffer, clientIP->h_name, inet_ntoa(*IP_A));

		char stringCopy[25];
		strcpy(stringCopy, echoBuffer);
		stringCopy[recvMsgSize-1] = '\0';

		//Processing the request
		struct message msg;
		convert_message(echoBuffer, &msg);
		operation(&msg);
		result_to_string(stringCopy, &msg);

		//Sending result to Client
		if (sendto(sock, stringCopy, strlen(stringCopy), 0, (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != strlen(stringCopy)){
			ErrorHandler("sendto() sent different number of bytes than expected");
			closesocket(sock);
			ClearWinSock();
			system("pause");
			return -1;
		}
		printf("Answer sent.\n\n");
	}

	closesocket(sock);
	ClearWinSock();
	system("pause");
	return 0;
}
