/*
 * Client.h
 *
 *  Created on: 3 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_CLIENT_H_
#define INCLUDE_CLIENT_H_

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef __linux__
#include <libgen.h>
#endif

#include "../BuildConfig.h"
#include "BasicTypes.h"
#include "BasicConstants.h"
#include "Common.h"
#include "md5.h"
#include "Serializer.h"

#ifdef DEBUG
int DEBUG_sendTestFile(char* serv_ip, int serv_port, char sendingfile_path[MAX_FULL_FILE_PATH_LEN], char* serv_pass);
#endif

int initTcpConnSocket(struct sockaddr_in* __tcpsocket_addr, int* __socket_desc, char* __server_addr, int __server_port);

#endif /* INCLUDE_CLIENT_H_ */
