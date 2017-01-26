/*
 * Common.c
 *
 *  Created on: 3 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Common.h"

char* getStrMsgType(log_msg_type_t _msg_type){
	switch(_msg_type){
	case LOG_INFO:
		return "INFO";
	case LOG_WARNING:
		return "WARNING";
	case LOG_ERROR:
		return "ERROR";
	}

	return "";
}

void logMsg(FILE* _stream_ptr, const char* _func_name, const int _line_number,
		log_msg_type_t _msg_type, const char *_format, ...){

	va_list ap;
	char msg[MAX_LOG_MSG_LEN];
	va_start(ap, _format);
	vsnprintf(msg, sizeof(msg), _format, ap);
	va_end(ap);

	char* curr_time_str = getCurrSysTimeStr();

#ifdef DEBUG
	fprintf(_stream_ptr, "[DEBUG-%s][%s(), {%d}][%s]:: %s\n", getStrMsgType(_msg_type), _func_name, _line_number, curr_time_str, msg);
#else
	fprintf(_stream_ptr, "[%s][%s]:: %s\n", getStrMsgType(_msg_type), curr_time_str, msg);
#endif

	fflush(_stream_ptr);
	free(curr_time_str);
}

char* getCurrSysTimeStr(){
	char* time_str = (char*) malloc(MAX_LOG_TIME_STR_LEN * sizeof(char) + sizeof(char));
	memset(time_str, '\0', MAX_LOG_TIME_STR_LEN * sizeof(char) + sizeof(char));
    struct tm *tm_strct;
    time_t time_now = time(0);

    tm_strct = gmtime(&time_now);
    strftime(time_str, MAX_LOG_TIME_STR_LEN * sizeof(char), "%Y.%m.%d-%H:%M:%S", tm_strct);

	return time_str;
}

file_size_t getFileSize(const char* _file_name){
	file_size_t file_size = 0;
#ifdef __linux__
	struct stat fileStatbuff;
	int fd = open(_file_name, O_RDONLY);
	if(fd == -1){
		file_size = -1;
	}
	else{
		if ((fstat(fd, &fileStatbuff) != 0) || (!S_ISREG(fileStatbuff.st_mode))) {
			file_size = -1;
		}
		else{
			file_size = fileStatbuff.st_size;
		}
		close(fd);
	}
#elif _WIN32
	HANDLE hFile;
	hFile = CreateFile(TEXT(_file_name), GENERIC_READ, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if(hFile == INVALID_HANDLE_VALUE){
		file_size = -1;
	}
	else{
		LARGE_INTEGER u_winFsz;
		GetFileSizeEx(hFile, &u_winFsz);
		CloseHandle(hFile);
		file_size = u_winFsz.QuadPart;
	}
#else
	FILE* fd = fopen(_file_name, "rb");
	if(fd == NULL){
		file_size = -1;
	}
	else{
		while(fgetc(fd) != -1)
			file_size++;
		fclose(fd);
	}

#endif
	return file_size;
}

char* getFileNameFromPath(const char* _file_path){
	char* file_name_ptr = NULL;
	char path_sep_symbol;

#ifdef __linux__
	path_sep_symbol = '/';
#elif _WIN32
	path_sep_symbol = '\\';
#else
	path_sep_symbol = '/';
#endif

	char *s = strrchr(_file_path, path_sep_symbol);
	if(s == NULL){
		file_name_ptr = strdup(_file_path);
	}
	else{
		file_name_ptr = strdup(s + 1);
	}
	return file_name_ptr;
}

/**
 * TODO: make version for _WIN32
 */
int countFileHash_md5(const char* _full_file_name, OUT_ARG BYTE* _file_hash){
	int file_d = 0;

	char* data_buff = NULL;
	file_size_t bytes_readed = 0;
	MD5_CTX ctx;

	// init md5 hasher
	md5_init(&ctx);

	// open file for sending
	file_d = open(_full_file_name, O_RDONLY);
	if(-1 == file_d){
		return EXIT_FAILURE;
	}

	data_buff = (char*) malloc((SENDING_FILE_PACKET_SIZE) * sizeof(char));
	memset(data_buff, '\0', (SENDING_FILE_PACKET_SIZE) * sizeof(char));

	// read file bytes block and update md5 hash-counter
	while( ((bytes_readed = read(file_d, data_buff, SENDING_FILE_PACKET_SIZE * sizeof(char))) > 0) ){
		//logMsg(__func__, __LINE__, LOG_INFO, "md5 bytes_readed: %d", bytes_readed);
		md5_update(&ctx, data_buff, bytes_readed);
		memset(data_buff, '\0', (SENDING_FILE_PACKET_SIZE) * sizeof(char));
	}

	md5_final(&ctx, _file_hash);

	close(file_d);
	return EXIT_SUCCESS;
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
