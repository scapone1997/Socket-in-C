/*
 * protocol.h
 *
 *  Created on: 15 nov 2021
 *      Author: simoc
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define PROTOPORT 27015 // default protocol port number

struct message{
	char operation;
	int first;
	int second;
	float result;
	int error;
};

void print_message(struct message* mess){
	printf("Operation: %c\n", mess->operation);
	printf("First: %d\n", mess->first);
	printf("Second: %d\n", mess->second);
	printf("Result: %.2f\n", mess->result);
	printf("Error: %d\n", mess->error);
}


#endif /* PROTOCOL_H_ */
