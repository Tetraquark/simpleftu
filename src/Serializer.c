/*
 * Serializer.c
 *
 *  Created on: 1 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Serializer.h"

ssize_t serialize_FileInfoMsg(file_info_msg_t _inStruct, const char _delimSymbol, OUT_ARG char** _out_fileInfoMsg){
	int fileNameStr_len = strlen(_inStruct.fileName);

	if(fileNameStr_len <= 0 || _inStruct.fileSize <= 0)
		return -1;

	char fileSizeMsgStr[MAX_FILESIZE_CHAR_NUM];
	memset(fileSizeMsgStr, 0, MAX_FILESIZE_CHAR_NUM);
	sprintf(fileSizeMsgStr, "%" PRId64, _inStruct.fileSize);

	int fileSizeStr_len = strlen(fileSizeMsgStr);

	ssize_t buffSize = sizeof(char) * MAX_FILESIZE_CHAR_NUM + sizeof(char) * MAX_FILENAME_LEN + 1;
			//fileNameStr_len * sizeof(char) +
			//fileSizeStr_len * sizeof(char) +
			//2 * sizeof(char) + 1;

	char* serializedMsg = (char*) malloc(buffSize);
	memset(serializedMsg, '\0', buffSize);

	// paste fileName field
	strncpy(serializedMsg, _inStruct.fileName, fileNameStr_len * sizeof(char));
	// paste delim symbol
	strncpy(&serializedMsg[fileNameStr_len], &_delimSymbol, sizeof(char));
	// paste fileSize field
	strncpy(&serializedMsg[fileNameStr_len + 1], fileSizeMsgStr, fileSizeStr_len * sizeof(char));
	//strncpy(&serializedMsg[fileNameStr_len + 1 + fileSizeStr_len + 1], &_delimSymbol, sizeof(char));

	*_out_fileInfoMsg = serializedMsg;
	return buffSize;
}

int deserialize_FileInfoMsg(OUT_ARG file_info_msg_t* _outStruct, char* _msgBuff, const char _delimSymbol){
	if(_outStruct == NULL || _msgBuff == NULL)
		return EXIT_FAILURE;

	// get fileName field
	char* pch = strtok(_msgBuff, &_delimSymbol);
	if(pch == NULL)
		return EXIT_FAILURE;
	strncpy(_outStruct->fileName, pch, strlen(pch) * sizeof(char));

	// get fileSize field
	pch = strtok(NULL, &_delimSymbol);
	if(pch == NULL)
		return EXIT_FAILURE;
	_outStruct->fileSize = atoll(pch);

	// get fileHash_md5 field
	/*
	pch = strtok(_msgBuff, &_delimSymbol);
	if(pch == NULL)
		return EXIT_FAILURE;
	BYTE fileHash_md5_arr[MD5_BLOCK_SIZE];
	memset(fileHash_md5_arr, 0, MD5_BLOCK_SIZE * sizeof(BYTE));
	fromHexStrToByteArr(pch, MD5_BLOCK_SIZE * 2, fileHash_md5_arr);
	memcpy(_outStruct->fileHash_md5, fileHash_md5_arr, MD5_BLOCK_SIZE * sizeof(BYTE));
	*/

	return EXIT_SUCCESS;
}

int parse_ipaddrStrToParts(char* _addr_str, OUT_ARG char** _ip_str, OUT_ARG int* _port){
    char* pch = strtok(_addr_str, ":");
    if(pch == NULL)
    	return EXIT_FAILURE;
    strncpy(*_ip_str, pch, strlen(pch) * sizeof(char));

    pch = strtok(NULL, ":");
    if(pch == NULL)
    	return EXIT_FAILURE;
    (*_port) = atoi(pch);

	return EXIT_SUCCESS;
}

