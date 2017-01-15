/*
 * Server.h
 *
 *  Created on: 8 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#ifdef __linux__
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#elif _WIN32

#endif

#include "../BuildConfig.h"
#include "BasicTypes.h"
#include "BasicConstants.h"
#include "md5.h"
#include "Common.h"
#include "Serializer.h"
#include "Crypto.h"

int startServTCPListener(serverConfig_t* serverConf_ptr);

void* startListenTCPSocket(void* threadData);

void* startPeerThread(void* threadData);

int createServTCPSocket(struct sockaddr_in* _tcpsocket_addr, int* _socket_desc, int _port);

// stopgap measure
bool_t checkPassword(const char* servPassStr, const char* inputPassStr);

ssize_t recvData(int _socked_fd, size_t _recv_data_size, OUT_ARG char** _recv_buff);

#endif /* INCLUDE_SERVER_H_ */
