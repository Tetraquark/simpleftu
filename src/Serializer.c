/*
 * Serializer.c
 *
 *  Created on: 1 янв. 2017 г.
 *      Author: tetraquark
 */

#include "../include/Serializer.h"

char* serialize_FileInfoMsg(file_info_msg_t inStruct, const char delimSymbol){
	int token1_len = strlen(inStruct.fileName);

	if(token1_len <= 0 || inStruct.fileSize <= 0)
		return NULL;

	char fileSizeMsgStr[MAX_FILESIZE_CHAR_NUM];
	memset(fileSizeMsgStr, 0, MAX_FILESIZE_CHAR_NUM);
	sprintf(fileSizeMsgStr, "%lld", inStruct.fileSize);
	int token2_len = strlen(fileSizeMsgStr);

	size_t buffSize = token1_len * sizeof(char) + token2_len * sizeof(char) + sizeof(char);
	char* serializedMsg = (char*) malloc(buffSize);

	strncpy(serializedMsg, inStruct.fileName, token1_len);
	strncpy(&serializedMsg[token1_len], &delimSymbol, 1);
	strncpy(&serializedMsg[token1_len + 1], fileSizeMsgStr, token2_len);

	return serializedMsg;
}

int deserialize_FileInfoMsg(const file_info_msg_t* outStruct, char* msgBuff, const char delimSymbol){
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

	return EXIT_SUCCESS;
}

