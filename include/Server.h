/*
 * Server.h
 *
 *  Created on: 8 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
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

#include <fcntl.h>

#include "../BuildConfig.h"
#include "BasicConstants.h"
#include "Common.h"
#include "Serializer.h"
#include "Crypto.h"

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
	int port;
	serverMode_t mode;
} serverConfig_t;

typedef struct{
	int socketFd;
	int inputCommsPipeFd;
	serverConfig_t conf;
} serverSysInfo_t;


int startServTCPListener(serverConfig_t _serverConf);

void* startListenTCPSocket(void* threadData);

void* startPeerThread(void* threadData);

int createServTCPSocket(struct sockaddr_in* _tcpsocket_addr, int* _socket_desc, int _port);

// stopgap measure
bool_t checkPassword(const char* servPassStr, const char* inputPassStr);

#endif /* INCLUDE_SERVER_H_ */
