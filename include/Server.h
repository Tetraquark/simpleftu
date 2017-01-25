/*
 * Server.h
 *
 *  Created on: 8 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include "../BuildConfig.h"
#include "BasicTypes.h"
#include "BasicConstants.h"
#include "md5.h"
#include "Common.h"
#include "Network.h"
#include "Serializer.h"
#include "Crypto.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#ifdef __linux__
#include <sys/stat.h>
#include <pthread.h>
#elif _WIN32
#include <windows.h>
#include <ws2tcpip.h>
#endif

int startServTCPListener(serverConfig_t* _serv_conf_ptr);

thread_rc_t startListenTCPSocket(void* _thread_data_strc);

thread_rc_t startPeerThread(void* _thread_data_strc);

// stopgap measure
bool_t checkPassword(const char* _serv_passw_str, const char* _in_passw_str);

#endif /* INCLUDE_SERVER_H_ */
