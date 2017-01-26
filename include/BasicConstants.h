/*
 * BasicConstants.h
 *
 *  Created on: 6 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_BASICCONSTANTS_H_
#define INCLUDE_BASICCONSTANTS_H_

#include <limits.h>

#include "../BuildConfig.h"

/**
 *
 */
#define OUT_ARG

#define MAX_LOG_MSG_LEN 1024
#define MAX_LOG_TIME_STR_LEN 19

#ifdef DEBUG
#define DEFAULT_SERVER_PORT DEBUG_SERVER_PORT
#else
#define DEFAULT_SERVER_PORT 10888
#endif

#ifdef DEBUG
#define DEFAULT_CONFFILE_NAME DEBUG_CONFFILE_NAME
#else
#define DEFAULT_CONFFILE_NAME "config.cfg"
#endif

#ifdef DEBUG
#define DEFAULT_STORAGEDIR_NAME DEBUG_SERVER_FILES_STORAGE_PATH
#else
#define DEFAULT_STORAGEDIR_NAME "sftu_storage"
#endif

#define DEFAULT_LOGFILE_PATH "logs.txt"

/*
 * MAX_FILE_SIZE
 * Maximum sending file size;
 */
#ifdef DEBUG
// 32 Gb
#define MAX_FILE_SIZE 34359738368
#define MAX_FILESIZE_CHAR_NUM 11
#else
#define MAX_FILE_SIZE 34359738368
#define MAX_FILESIZE_CHAR_NUM 11
#endif

/*
 * MAX_PASS_LEN
 * Maximum password length in char symbols;
 */
#define MAX_PASS_LEN 8

#ifdef FILENAME_MAX
#define MAX_FILENAME_LEN FILENAME_MAX
#else
#define MAX_FILENAME_LEN 260
#endif

#define MAX_FULL_FILE_PATH_LEN MAX_FILENAME_LEN + 128

#define MAX_STORAGEDIR_PATH_LEN MAX_FULL_FILE_PATH_LEN + MAX_FILENAME_LEN

#define SENDING_FILE_PACKET_SIZE 4096

#define IPV4_ADDR_STR_LEN 16
#define IPADDR_STR_LEN IPV4_ADDR_STR_LEN

#endif /* INCLUDE_BASICCONSTANTS_H_ */
