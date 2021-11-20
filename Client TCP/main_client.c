/*
 * CLIENT TCP
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
#define BUFFERSIZE 512

void insert_string(char* str, int length){

	int number = 1;
	while(number == 1){
		printf("Insert message in the form [Operation Integer Integer]: \n");
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

}

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

void close_message(struct message* mess){

	mess->operation = '=';
	mess->first = 0;
	mess->second = 0;
	mess->result = 0;
	mess->error = 0;
}

void errorhandler(char *error_message) {
	printf("%s",error_message);
}

void clearwinsock() {
	#if defined WIN32
	WSACleanup();
	#endif
}


int main(int argc, char* argv[]) {
	#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2 ,2), &wsa_data);
	if (result != 0) {
		printf ("error at WSASturtup\n");
		return -1;
	}
	#endif


	char* serverAddress;
	int serverPort;

	if(argc > 1){
		strcpy(serverAddress, argv[1]);
		serverPort = atoi(argv[2]);
	}
	else
	{
		strcpy(serverAddress, "127.0.0.1");
		serverPort = PROTOPORT;
	}

	//Creating Socket
	int c_socket;
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket < 0) {
		errorhandler("socket creation failed.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	//Building the Server Address
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(serverAddress); // IP del server
	sad.sin_port = htons(serverPort); // Server port


	//Connecting to Server...
	if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad))< 0){
		errorhandler( "Failed to connect.\n" );
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	//We manage client requests by forwarding them to server..
	while(1){
		struct message mess;
		char str[25];

		//Asking user to insert a string in the form (Operation Integer Integer)..
		insert_string(str, 25);

		if(str[0] == '='){
			close_message(&mess);
			//Sending message to Server
			if (send(c_socket, (char*)&mess, sizeof(mess), 0) != sizeof(mess)) {
				errorhandler("send() sent a different number of bytes thanexpected");
				closesocket(c_socket);
				clearwinsock();
				return -1;
			}
			break;
		}

		//Converting the string into a message for the Server
		convert_message(str, &mess);

		//Sending message to Server
		if (send(c_socket, (char*)&mess, sizeof(mess), 0) != sizeof(mess)) {
			errorhandler("send() sent a different number of bytes thanexpected");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

		//Receiving the answer for the operation..
		if ((recv(c_socket, (char*)&mess, sizeof(mess), 0)) <= 0) {
			errorhandler("recv() failed or connection closed prematurely");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

		//Checking if result contains error...
		if(mess.error == 0)
			printf("The result is: %.2f\n", mess.result);
		else printf("The result is: error\n");
	}

	closesocket(c_socket);
	clearwinsock();

	system("pause");
	return(0);
}
