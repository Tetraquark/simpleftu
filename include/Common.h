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

#include "../BuildConfig.h"
#include "BasicConstants.h"
#include "BasicTypes.h"

#ifdef DEBUG
void DEBUG_printlnStdoutMsg(const char* __func_name__, const int __line_number__, log_msg_type_t msg_type, const char* debug_msg);
#endif

char* getStrMsgType(log_msg_type_t msg_type);
void logMsg(const char* __func_name__, const int __line_number__, log_msg_type_t msg_type, const char *format, ...);

#endif /* INCLUDE_COMMON_H_ */
