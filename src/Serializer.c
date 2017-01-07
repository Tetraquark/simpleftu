/*
 * Serializer.c
 *
 *  Created on: 1 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Serializer.h"

ssize_t serialize_FileInfoMsg(file_info_msg_t inStruct, const char delimSymbol, OUT_ARG char** out_fileInfoMsg){
	int _fileNameStr_len = strlen(inStruct.fileName);

	if(_fileNameStr_len <= 0 || inStruct.fileSize <= 0)
		return -1;

	char _fileSizeMsgStr[MAX_FILESIZE_CHAR_NUM];
	memset(_fileSizeMsgStr, 0, MAX_FILESIZE_CHAR_NUM);
#ifdef __linux__
	sprintf(_fileSizeMsgStr, "%lld", inStruct.fileSize);
#endif
	int _fileSizeStr_len = strlen(_fileSizeMsgStr);

	ssize_t _buffSize = _fileNameStr_len * sizeof(char) +
			_fileSizeStr_len * sizeof(char) +
			//fileHashMd5Str_len * sizeof(char) +
			2 * sizeof(char);

	char* _serializedMsg = (char*) malloc(_buffSize);

	// paste fileName field
	strncpy(_serializedMsg, inStruct.fileName, _fileNameStr_len * sizeof(char));
	// paste delim symbol
	strncpy(&_serializedMsg[_fileNameStr_len], &delimSymbol, sizeof(char));
	// paste fileSize field
	strncpy(&_serializedMsg[_fileNameStr_len + 1], _fileSizeMsgStr, _fileSizeStr_len * sizeof(char));
	// paste delim symbol
	//memcpy(&_serializedMsg[_fileNameStr_len + 1 + _fileSizeStr_len], &delimSymbol, sizeof(char));
	// paste fileHash_md5 field
	//memcpy(&_serializedMsg[_fileNameStr_len + 1 + _fileSizeStr_len + 1], fileMd5HashStr, fileHashMd5Str_len * sizeof(char));
	strncpy(&_serializedMsg[_fileNameStr_len + 1 + _fileSizeStr_len + 1], &delimSymbol, sizeof(char));

	*out_fileInfoMsg = _serializedMsg;
	return _buffSize;
}

int deserialize_FileInfoMsg(OUT_ARG file_info_msg_t* outStruct, char* msgBuff, const char delimSymbol){
	if(outStruct == NULL || msgBuff == NULL)
		return EXIT_FAILURE;

	// get fileName field
	char* _pch = strtok(msgBuff, &delimSymbol);
	if(_pch == NULL)
		return EXIT_FAILURE;
	strncpy(outStruct->fileName, _pch, strlen(_pch) * sizeof(char));

	// get fileSize field
	_pch = strtok(NULL, &delimSymbol);
	if(_pch == NULL)
		return EXIT_FAILURE;
	outStruct->fileSize = atoll(_pch);

	// get fileHash_md5 field
	/*
	_pch = strtok(msgBuff, &delimSymbol);
	if(_pch == NULL)
		return EXIT_FAILURE;
	BYTE fileHash_md5_arr[MD5_BLOCK_SIZE];
	memset(fileHash_md5_arr, 0, MD5_BLOCK_SIZE * sizeof(BYTE));
	fromHexStrToByteArr(_pch, MD5_BLOCK_SIZE * 2, fileHash_md5_arr);
	memcpy(outStruct->fileHash_md5, fileHash_md5_arr, MD5_BLOCK_SIZE * sizeof(BYTE));
	*/

	return EXIT_SUCCESS;
}

int parse_ipaddrStrToParts(char* addr_str, OUT_ARG char** ip_str, OUT_ARG int* port){
	char delimSymbol = ':';

    char* _pch = strtok(addr_str, &delimSymbol);
    if(_pch == NULL)
    	return EXIT_FAILURE;
    strncpy(*ip_str, _pch, strlen(_pch) * sizeof(char));

    _pch = strtok(NULL, &delimSymbol);
    if(_pch == NULL)
    	return EXIT_FAILURE;
    (*port) = atoi(_pch);

	return EXIT_SUCCESS;
}

