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

file_size_t getFileSize(const char* file_name){
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
	HANDLE hFile;
	hFile = CreateFile(TEXT(file_name), GENERIC_READ, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if(hFile == INVALID_HANDLE_VALUE){
		_file_size = -1;
	}
	else{
		LARGE_INTEGER u_winFsz;
		GetFileSizeEx(hFile, &u_winFsz);
		CloseHandle(hFile);
		_file_size = u_winFsz.QuadPart;
	}
#else
	FILE* fd = fopen(file_name, "rb");
	if(fd == NULL){
		_file_size = -1;
	}
	else{
		while(fgetc(fd) != -1)
			_file_size++;
		fclose(fd);
	}

#endif
	return _file_size;
}

char* getFileNameFromPath(const char* file_path){
	char* _file_name_ptr = NULL;
	char _path_sep_symbol;

#ifdef __linux__
	_path_sep_symbol = '/';
#elif _WIN32
	_path_sep_symbol = '\\';
#else
	_path_sep_symbol = '/';
#endif

	char *s = strrchr(file_path, _path_sep_symbol);
	if(s == NULL){
		_file_name_ptr = strdup(file_path);
	}
	else{
		_file_name_ptr = strdup(s + 1);
	}
	return _file_name_ptr;
}

/**
 * TODO: make version for _WIN32
 */
int countFileHash_md5(const char* full_file_name, OUT_ARG BYTE* file_hash){
	int fd = 0;
	char* fd_buff = NULL;
	file_size_t bytes_readed = 0;
	MD5_CTX ctx;

	// init md5 hasher
	md5_init(&ctx);

	// open file for sending
	fd = open(full_file_name, O_RDONLY);
	if(fd == -1){
		return EXIT_FAILURE;
	}

	fd_buff = (char*) malloc((SENDING_FILE_PACKET_SIZE) * sizeof(char));
	memset(fd_buff, '\0', (SENDING_FILE_PACKET_SIZE) * sizeof(char));

	// read file bytes block and update md5 hash-counter
	while( ((bytes_readed = read(fd, fd_buff, SENDING_FILE_PACKET_SIZE * sizeof(char))) > 0) ){
		md5_update(&ctx, fd_buff, bytes_readed);
		memset(fd_buff, '\0', (SENDING_FILE_PACKET_SIZE) * sizeof(char));
	}

	md5_final(&ctx, file_hash);

	return EXIT_SUCCESS;
}

int closeSocket(socket_t _socket_d){
	int rc = EXIT_FAILURE;
#ifdef _WIN32
	rc = closesocket(_socket_d);
#elif __linux__
	rc = close(_socket_d);
#else
#endif
	return rc;
}

void exitThread(thread_rc_t _exit_arg){
#ifdef _WIN32
	ExitThread(_exit_arg);
#elif __linux__
	pthread_exit(_exit_arg);
#else
#endif
}

void joinThread_inf(thread_t _thread_d){
#ifdef _WIN32
		WaitForSingleObject(_thread_d, INFINITE);
#elif __linux__
		pthread_join(_thread_d, NULL);
#endif
}
