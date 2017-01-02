/*
 * Server.h
 *
 *  Created on: 8 окт. 2016 г.
 *      Author: tetraquark
 */

#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "../BuildConfig.h"
#include "BasicConstants.h"
#include "Serializer.h"

typedef enum{
	LISTEN,
	STOP_SERVER
} serverCommands_t;

typedef enum{
	SINGLE_CLIENT,
	MULTI_CLIENT
} serverMode_t;

typedef struct{
	char password[MAX_PASS_LEN];
	char* saveFilesFolder;
} serverConfig_t;

typedef struct{
	int socketFd;
	int inputCommsPipeFd;
	serverMode_t mode;
	serverConfig_t conf;
} serverSysInfo_t;


int startTCPServer();

void* startListenTCPSocket(void* threadData);

int createServTCPSocket(struct sockaddr_in* _tcpsocket_addr, int* _socket_desc, int _port);

#endif /* INCLUDE_SERVER_H_ */
