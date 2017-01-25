/*
 * Network.c
 *
 *  Created on: 24 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Network.h"

int socket_close(socket_t _socket_d){
	int rc = EXIT_FAILURE;
#ifdef _WIN32
	rc = closesocket(_socket_d);
#elif __linux__
	rc = close(_socket_d);
#else
#endif
	return rc;
}

ssize_t socket_sendBytes(socket_t _socket_d, const void* _buff, size_t _size){
	ssize_t bytes_sent = send(_socket_d, _buff, _size, 0);
	logMsg(__func__, __LINE__, LOG_INFO, "Sent bytes: %d", bytes_sent);
	return bytes_sent ;
}

ssize_t socket_recvBytes(socket_t _socket_d, size_t _recv_size, OUT_ARG void* _recv_buff){
	ssize_t total_recved_bytes = 0;
	ssize_t bytes_readed = 0;

	while(total_recved_bytes < _recv_size){
		if((bytes_readed = recv(_socket_d, _recv_buff + total_recved_bytes, _recv_size - total_recved_bytes, 0)) <= 0){
			total_recved_bytes = -1;
			break;
		}
		total_recved_bytes += bytes_readed;
	}

	logMsg(__func__, __LINE__, LOG_INFO, "Received bytes: %d", total_recved_bytes);
	return total_recved_bytes;
}

int socket_createServTCP(int _port, OUT_ARG struct sockaddr_in* _tcpsocket_addr, OUT_ARG socket_t* _socket_desc){

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
		return EXIT_FAILURE;
#endif

	*_socket_desc = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
	if(INVALID_SOCKET == *_socket_desc)
#elif __linux__
	if(*_socket_desc < 0)
#else
#endif
		return EXIT_FAILURE;

    memset(_tcpsocket_addr, 0, sizeof(*_tcpsocket_addr));
    (*_tcpsocket_addr).sin_family      = AF_INET;
    (*_tcpsocket_addr).sin_addr.s_addr = htonl(INADDR_ANY);
    (*_tcpsocket_addr).sin_port        = htons(_port);

#ifdef _WIN32
    if( bind(*_socket_desc, (struct sockaddr *)_tcpsocket_addr, sizeof(*_tcpsocket_addr)) == SOCKET_ERROR )
#elif __linux__
    if( bind(*_socket_desc, (struct sockaddr *)_tcpsocket_addr, sizeof(*_tcpsocket_addr)) != 0 )
#endif
    	return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int socket_createPeerTCP(char* _server_addr, int _server_port, OUT_ARG struct sockaddr_in* _tcpsocket_addr, OUT_ARG socket_t* _socket_desc){
	logMsg(__func__, __LINE__, LOG_INFO, "Try to create socket.");
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
		return EXIT_FAILURE;
#endif

	*_socket_desc = socket(AF_INET , SOCK_STREAM, IPPROTO_TCP);

#ifdef _WIN32
	if(INVALID_SOCKET == *_socket_desc){
		WSACleanup();
#elif __linux__
	*_socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(*_socket_desc < 0){
#else
#endif
		return EXIT_FAILURE;
	}

	logMsg(__func__, __LINE__, LOG_INFO, "Socket created.");

    (*_tcpsocket_addr).sin_family      = AF_INET;
    (*_tcpsocket_addr).sin_addr.s_addr = inet_addr(_server_addr);
    (*_tcpsocket_addr).sin_port        = htons(_server_port);

    return EXIT_SUCCESS;
}
