/*
 * SERVER TCP
 *
 *  Created on: 25 ott 2021
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
#include <stdlib.h>
#include <string.h>
#include "protocol.h"
#define QLEN 5 // size of request queue

void errorhandler(char *errorMessage) {
	printf ("%s", errorMessage);
}

void clearwinsock() {
	#if defined WIN32
	WSACleanup();
	#endif
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

int main() {

	#if defined WIN32 // initialize Winsock

	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != 0) {
		errorhandler("Error at WSAStartup()\n");
		return 0;
	}
	#endif

	//Creation of the socket
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		errorhandler("socket creation failed.\n");
		clearwinsock();
		return -1;
	}

	//Giving an address to socket
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad)); // ensures that extra bytes contain 0
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(PROTOPORT); /* converts values between the host and
	network byte order. Specifically, htons() converts 16-bit quantities
	from host byte order to network byte order. */
	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("bind() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	//Setting socket to listen
	if (listen (my_socket, QLEN) < 0) {
		errorhandler("listen() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	//Accepting  new connection
	struct sockaddr_in cad; // structure for the client address
	int client_socket; // socket descriptor for the client
	int client_len; // the size of the client address
	printf("In attesa di richiesta di connessione da parte di un Client...");
	while (1) { /* oppure for (;;) */
		client_len = sizeof(cad); // set the size of the client address
		if ((client_socket = accept(my_socket, (struct sockaddr*)&cad, &client_len)) < 0) {
			errorhandler("accept() failed.\n");
			//Closure of connection
			closesocket(client_socket);
			clearwinsock();
			return 0;
		}

		printf("\nConnection established with: %s:", inet_ntoa(cad.sin_addr));
		printf("%u", cad.sin_port);
		printf("\n");

		while(1){
			int bytesRcvd;
			struct message mess;

			//Receving the message from Client
			if ((bytesRcvd = recv(client_socket, (char*)&mess, sizeof(mess), 0)) <= 0) {
				errorhandler("recv() failed or connection closed prematurely");
				closesocket(client_socket);
				clearwinsock();
				return -1;
			}
			printf("Received. \n");


			//If Client want to close connection..
			if(mess.operation == '=')
				break;

			//Computing the result...
			operation(&mess);
			printf("I'm computing...\n");

			//Sending answer to Client
			if (send(client_socket, (char*)&mess, sizeof(mess), 0) != sizeof(mess)) {
				errorhandler("send() sent a different number of bytes thanexpected");
				closesocket(client_socket);
				clearwinsock();
				return -1;
			}
			printf("Answer sent.\n");
		}

		closesocket(client_socket);
		printf("Connection closed.\n\n");
	} // end-while
	return(0);
} // end-main

