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

file_size_t getFileSize(char* file_name){
	file_size_t _file_size = 0;
#ifdef __linux__
	struct stat _fileStatbuff;
	int fd = open(file_name, O_RDONLY);
	if(fd == -1){
		_file_size = -1;
	}
	else{
		if ((fstat(fd, &_fileStatbuff) != 0) || (!S_ISREG(_fileStatbuff.st_mode))) {
			_file_size = -1;
		}
		else{
			_file_size = _fileStatbuff.st_size;
		}
		close(fd);
	}
#elif _WIN32

#else
	FILE* fd = fopen(file_name, "r");
	if(fd == NULL){
		_file_size = -1;
	}
	else{
		while(getc(fd) != EOF)
			_file_size++;
		fclose(fd);
	}

#endif
	return _file_size;
}
