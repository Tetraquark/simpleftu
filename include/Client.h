/*
 * Client.h
 *
 *  Created on: 3 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_CLIENT_H_
#define INCLUDE_CLIENT_H_

#include "../BuildConfig.h"
#include "BasicTypes.h"
#include "BasicConstants.h"
#include "Network.h"
#include "Common.h"
#include "md5.h"
#include "Serializer.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef __linux__
#include <libgen.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

int startClient(char* _serv_ip, int _serv_port, char _sendingfile_path[MAX_FULL_FILE_PATH_LEN + 1], char* _serv_pass);

file_size_t sendFile(int _socket, const char* _full_file_name);

#endif /* INCLUDE_CLIENT_H_ */
