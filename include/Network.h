/*
 * Network.h
 *
 *  Created on: 24 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_NETWORK_H_
#define INCLUDE_NETWORK_H_

#include "../BuildConfig.h"
#include "BasicTypes.h"
#include "BasicConstants.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#elif __linux__
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#endif

int socket_close(socket_t _socket_d);

ssize_t socket_sendBytes(socket_t _socket_d, const void* _buff, size_t _size);

ssize_t socket_recvBytes(socket_t _socket_d, size_t _recv_size, OUT_ARG void* _recv_buff);

/**
 * Create "server" TCP socket for _socket_desc descriptor and bind into _port.
 * @return 0 success; 1 error
 */
int socket_createServTCP(int _port, OUT_ARG struct sockaddr_in* _tcpsocket_addr, OUT_ARG socket_t* _socket_desc);

/**
 * Create "peer" TCP socket for _socket_desc descriptor to connect to server
 * by __server_addr address and _server_port port.
 * @return 0 success; 1 error
 */
int socket_createPeerTCP(char* _server_addr, int _server_port, OUT_ARG struct sockaddr_in* _tcpsocket_addr, OUT_ARG int* _socket_desc);

#endif /* INCLUDE_NETWORK_H_ */
