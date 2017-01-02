/*
 * Crypto.c
 *
 *  Created on: 2 янв. 2017 г.
 *      Author: tetraquark
 */

#include "../include/Crypto.h"

int fromHexStrToByteArr(const char* hexStr, int hexStrSize, BYTE* out_byteArr){
	if(out_byteArr == NULL || hexStr == NULL || hexStrSize % 2 != 0)
		return EXIT_FAILURE;

	memset(out_byteArr, '\0', hexStrSize / 2 * sizeof(char));
	const char* pos = hexStr;

	for(int i = 0; i < hexStrSize; i++){
		sscanf(pos, "%2hhx", &out_byteArr[i]);
		pos += 2;
	}

	return EXIT_SUCCESS;
}

int fromByteArrToHexStr(const BYTE* byteArr, int byteArrSize, char* out_strArr){
	if(out_strArr == NULL || out_strArr == NULL || byteArrSize % 2 != 0)
		return EXIT_FAILURE;

	memset(out_strArr, '\0', byteArrSize * 2 * sizeof(char));

	for(int i = 0; i < byteArrSize; i++){
		sprintf(out_strArr + i * 2, "%02x", byteArr[i]);
	}

	return EXIT_SUCCESS;
}

int md5HashCmp(const BYTE* hash1, const BYTE hash2){
	return memcmp(hash1, hash2, MD5_BLOCK_SIZE * sizeof(BYTE));
}
