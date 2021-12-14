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
#define ECHOMAX 255
#define PORT 48000

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
	struct sockaddr_in echoServAddr;
	struct sockaddr_in fromAddr;
	unsigned int fromSize;
	char echoBuffer[ECHOMAX];
	int echoStringLen;
	int respStringLen;
	char serverAddress[25];
	int serverPort;

	// CREAZIONE DELLA SOCKET
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		ErrorHandler("socket() failed");

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

		if(strcmp(uno, "localhost") == 0)
			strcpy(serverAddress, "127.0.0.1");
		serverPort = atoi(due);
	}
	else
	{
		//otherwise we use the default values
		strcpy(serverAddress, "127.0.0.1");
		serverPort = PORT;
	}

	// COSTRUZIONE DELL'INDIRIZZO DEL SERVER
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

		// INVIO DELLA STRINGA ECHO AL SERVER
		if (sendto(sock, string, echoStringLen, 0, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) != echoStringLen)
			ErrorHandler("sendto() sent different number of bytes than expected");

		// RITORNO DELLA STRINGA ECHO
		fromSize = sizeof(fromAddr);
		respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr*)&fromAddr, &fromSize);
		echoBuffer[respStringLen] = '\0';
		printf("Ricevuto risultato dal server %s ip %s: %s\n", "localhost", "127.0.0.1", echoBuffer);

		if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
			fprintf(stderr, "Error: received a packet from unknown source.\n");
			exit(EXIT_FAILURE);
		}

	}

	closesocket(sock);
	ClearWinSock();
	system("pause");
	return 0;
}
