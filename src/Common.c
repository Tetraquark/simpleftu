/*
 * Common.c
 *
 *  Created on: 3 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Common.h"

#ifdef DEBUG
void DEBUG_printlnStdoutMsg(const char* __func_name__, const int __line_number__, log_msg_type_t msg_type, const char* debug_msg){
	printf("[DEBUG-%s][%s(), {%d}]:: %s\n", getStrMsgType(msg_type), __func_name__, __line_number__, debug_msg);
	fflush(stdout);
}
#endif

char* getStrMsgType(log_msg_type_t msg_type){
	switch(msg_type){
	case INFO:
		return "INFO";
	case WARNING:
		return "WARNING";
	case ERROR:
		return "ERROR";
	}

	return "";
}

void logMsg(const char* __func_name__, const int __line_number__, log_msg_type_t msg_type, const char *format, ...){

	va_list ap;
	char msg[MAX_LOG_MSG_LEN];
	va_start(ap, format);
	vsnprintf(msg, sizeof(msg), format, ap);
	va_end(ap);

#ifdef DEBUG
	DEBUG_printlnStdoutMsg(__func_name__, __line_number__, msg_type, msg);
#else
#endif
}
