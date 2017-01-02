/*
 * Serializer.c
 *
 *  Created on: 1 янв. 2017 г.
 *      Author: tetraquark
 */

#include "../include/Serializer.h"

char* serialize_FileInfoMsg(file_info_msg_t inStruct, const char delimSymbol){
	int fileNameStr_len = strlen(inStruct.fileName);

	if(fileNameStr_len <= 0 || inStruct.fileSize <= 0)
		return NULL;

	char fileSizeMsgStr[MAX_FILESIZE_CHAR_NUM];
	memset(fileSizeMsgStr, 0, MAX_FILESIZE_CHAR_NUM);
	sprintf(fileSizeMsgStr, "%lld", inStruct.fileSize);
	int fileSizeStr_len = strlen(fileSizeMsgStr);

	char fileMd5HashStr[MD5_BLOCK_SIZE * 2];
	memset(fileMd5HashStr, '\0', MD5_BLOCK_SIZE * 2 * sizeof(char));
	if(!fromByteArrToHexStr(inStruct.fileHash_md5, MD5_BLOCK_SIZE, fileMd5HashStr)){
		return NULL;
	}

	int fileHashMd5Str_len = strlen(fileMd5HashStr);

	size_t buffSize = fileNameStr_len * sizeof(char) + fileSizeStr_len * sizeof(char) +
			fileHashMd5Str_len * sizeof(char) + 2 * sizeof(char);

	char* serializedMsg = (char*) malloc(buffSize);

	// paste fileName field
	memcpy(serializedMsg, inStruct.fileName, fileNameStr_len * sizeof(char));
	// paste delim symbol
	memcpy(&serializedMsg[fileNameStr_len], &delimSymbol, sizeof(char));
	// paste fileSize field
	memcpy(&serializedMsg[fileNameStr_len + 1], fileSizeMsgStr, fileSizeStr_len * sizeof(char));
	// paste delim symbol
	memcpy(&serializedMsg[fileNameStr_len + 1 + fileSizeStr_len], &delimSymbol, sizeof(char));
	// paste fileHash_md5 field
	memcpy(&serializedMsg[fileNameStr_len + 1 + fileSizeStr_len + 1], fileMd5HashStr, fileHashMd5Str_len * sizeof(char));

	return serializedMsg;
}

int deserialize_FileInfoMsg(file_info_msg_t* outStruct, char* msgBuff, const char delimSymbol){
	if(outStruct == NULL || msgBuff == NULL)
		return EXIT_FAILURE;

	// get fileName field
	char* pch = strtok(msgBuff, &delimSymbol);
	if(pch == NULL)
		return EXIT_FAILURE;
	strncpy(outStruct->fileName, pch, strlen(pch) * sizeof(char));

	// get fileSize field
	pch = strtok(msgBuff, &delimSymbol);
	if(pch == NULL)
		return EXIT_FAILURE;
	outStruct->fileSize = atoll(pch);

	// get fileHash_md5 field
	pch = strtok(msgBuff, &delimSymbol);
	if(pch == NULL)
		return EXIT_FAILURE;
	BYTE fileHash_md5_arr[MD5_BLOCK_SIZE];
	memset(fileHash_md5_arr, 0, MD5_BLOCK_SIZE * sizeof(BYTE));
	fromHexStrToByteArr(pch, MD5_BLOCK_SIZE * 2, fileHash_md5_arr);
	memcpy(outStruct->fileHash_md5, fileHash_md5_arr, MD5_BLOCK_SIZE * sizeof(BYTE));

	return EXIT_SUCCESS;
}

