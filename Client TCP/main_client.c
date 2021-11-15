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
#define BUFFERSIZE 512

struct message{
	char operation;
	int first;
	int second;
	float result;
	int error;
};


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

void print_message(struct message* mess){
	printf("Operation: %c\n", mess->operation);
	printf("First: %d\n", mess->first);
	printf("Second: %d\n", mess->second);
	printf("Result: %.2f\n", mess->result);
	printf("Error: %d\n", mess->error);
}

void errorhandler(char *error_message) {
	printf("%s",error_message);
}

void clearwinsock() {
	#if defined WIN32
	WSACleanup();
	#endif
}


int main(void) {
	#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2 ,2), &wsa_data);
	if (result != 0) {
		printf ("error at WSASturtup\n");
		return -1;
	}
	#endif

	//Asking user to insert Serve number port
	char* serverAddress;
	printf("Insert address of the Server:\n");
	scanf("%s", serverAddress);
	fflush(stdin);

	if(!strcmp(serverAddress, "127.0.0.1") == 0)
		strcpy(serverAddress, "127.0.0.1");

	//Asking user to insert Port number port
	int serverPort;
	printf("Insert port of the Server:\n");
	scanf("%d", &serverPort);
	fflush(stdin);

	if(serverPort != 27015)
		serverPort = 27015;

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

	//Asking the user to insert a message in the form (+ Integer Integer)
	while(1){
		struct message mess;
		char str[25];
		int i;
		printf("Insert message in the form (+ Integer Integer): \n");
		fgets(str, sizeof(str), stdin);
		fflush(stdin);

		i = strlen(str) - 1;
		if (str[i] == '\n')
		    str[i] = '\0';

		if(strcmp(str, "=") == 0){
			close_message(&mess);
			//Sending message to Server
			if (send(c_socket, &mess, sizeof(mess), 0) != sizeof(mess)) {
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
		if (send(c_socket, &mess, sizeof(mess), 0) != sizeof(mess)) {
			errorhandler("send() sent a different number of bytes thanexpected");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

		//Receiving the answer for the operation..
		if ((recv(c_socket, &mess, sizeof(mess), 0)) <= 0) {
			errorhandler("recv() failed or connection closed prematurely");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

		if(mess.error == 0)
			printf("The result is: %.2f\n", mess.result);
		else printf("The result is: error\n");
	}

	closesocket(c_socket);
	clearwinsock();

	system("pause");
	return(0);
}
