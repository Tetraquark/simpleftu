/*
 * Crypto.c
 *
 *  Created on: 2 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Crypto.h"

int fromHexStrToByteArr(const char* hexStr, int hexStr_size, OUT_ARG BYTE* out_byteArr){
	if(out_byteArr == NULL || hexStr == NULL)
		return EXIT_FAILURE;

	const char* pos = hexStr;

	for(int i = 0; i < hexStr_size; i++){
		sscanf(pos, "%2hhx", &out_byteArr[i]);
		pos += 2;
	}

	return EXIT_SUCCESS;
}

int fromByteArrToHexStr(const BYTE* byteArr, int byteArr_size, OUT_ARG char** out_strArr){
	if(out_strArr == NULL || out_strArr == NULL)
		return EXIT_FAILURE;

	for(int i = 0; i < byteArr_size; i++){
		sprintf(*out_strArr + i * 2, "%02x", byteArr[i]);
	}

	return EXIT_SUCCESS;
}

int cmpHash_md5(const BYTE hash1[MD5_BLOCK_SIZE], const BYTE hash2[MD5_BLOCK_SIZE]){
	return memcmp(hash1, hash2, MD5_BLOCK_SIZE * sizeof(BYTE));
}
