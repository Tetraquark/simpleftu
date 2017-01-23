/*
 * BasicTypes.h
 *
 *  Created on: 6 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_BASICTYPES_H_
#define INCLUDE_BASICTYPES_H_

#include <stdint.h>

#ifdef __linux__
#include <pthread.h>
#endif

#include "../BuildConfig.h"
#include "md5.h"
#include "BasicConstants.h"

typedef int64_t file_size_t;

#ifdef _WIN32
typedef SOCKET socket_t;
#elif __linux__
typedef int socket_t;
#else
#endif

/**
 * Type for threads descriptors.
 */
#ifdef _WIN32
typedef HANDLE thread_t;
#elif __linux__
typedef pthread_t thread_t;
#else
#endif

/**
 * Type for return value from threads.
 */
#ifdef _WIN32
typedef DWORD thread_rc_t;
#elif __linux__
typedef void* thread_rc_t;
#else
#endif

enum __bool{
	FALSE = 0,
	TRUE
};
typedef enum __bool bool_t;

enum __log_msg_type{
	INFO = 0,
	WARNING,
	ERROR
};
typedef enum __log_msg_type log_msg_type_t;

enum __netmsg_sending_res{
	SUCCESSFUL = 0,
	FAIL
};
typedef enum __netmsg_sending_res netmsg_sending_res_t;

enum __mode_type{
	MODE_NONE,
	MODE_DAEMON,
	MODE_SERVER,
	MODE_CLIENT,
};
typedef enum __mode_type mode_type_t;

struct __file_info_msg{
	file_size_t fileSize;					// in bytes
	char fileName[MAX_FILENAME_LEN];
	//BYTE fileHash_md5[MD5_BLOCK_SIZE];
};
typedef struct __file_info_msg file_info_msg_t;

enum __serverCommands{
	LISTEN,
	STOP_SERVER
};
typedef enum __serverCommands serverCommands_t;

struct __serverConfig{
	char* password;
	char* storageFolderPath;
	int port;
};
typedef struct __serverConfig serverConfig_t;

struct __serverSysInfo{
	socket_t socketFd;
	int inputCommsPipeFd;
	serverConfig_t* conf;
};
typedef struct __serverSysInfo serverSysInfo_t;

#endif /* INCLUDE_BASICTYPES_H_ */
