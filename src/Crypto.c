/*
 * Crypto.c
 *
 *  Created on: 2 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Crypto.h"

int fromHexStrToByteArr(const char* _hexStr, int _hexStr_size, OUT_ARG BYTE* _out_byteArr){
	if(_out_byteArr == NULL || _hexStr == NULL)
		return EXIT_FAILURE;

	const char* pos = _hexStr;

	for(int i = 0; i < _hexStr_size; i++){
		sscanf(pos, "%2hhx", &_out_byteArr[i]);
		pos += 2;
	}

	return EXIT_SUCCESS;
}

int fromByteArrToHexStr(const BYTE* _byteArr, int _byteArr_size, OUT_ARG char** _out_strArr){
	if(_out_strArr == NULL || _out_strArr == NULL)
		return EXIT_FAILURE;

	for(int i = 0; i < _byteArr_size; i++){
		sprintf(*_out_strArr + i * 2, "%02x", _byteArr[i]);
	}

	return EXIT_SUCCESS;
}

int cmpHash_md5(const BYTE _hash1[MD5_BLOCK_SIZE], const BYTE _hash2[MD5_BLOCK_SIZE]){
	return memcmp(_hash1, _hash2, MD5_BLOCK_SIZE * sizeof(BYTE));
}
