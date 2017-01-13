/*
 * Common.h
 *
 *  Created on: 3 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
// for getFileSize()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#elif _WIN32
#endif

#include "../BuildConfig.h"
#include "BasicConstants.h"
#include "BasicTypes.h"
#include "md5.h"

#ifdef DEBUG
void DEBUG_printlnStdoutMsg(const char* __func_name__, const int __line_number__, log_msg_type_t msg_type, const char* debug_msg);
#endif

char* getStrMsgType(log_msg_type_t msg_type);
void logMsg(const char* __func_name__, const int __line_number__, log_msg_type_t msg_type, const char *format, ...);

file_size_t getFileSize(const char* file_name);

char* getFileNameFromPath(const char* file_path);

int countFileHash_md5(const char* full_file_name, OUT_ARG BYTE* file_hash);

#endif /* INCLUDE_COMMON_H_ */
