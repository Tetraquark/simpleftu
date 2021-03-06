/*
 * Crypto.h
 *
 *  Created on: 2 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_CRYPTO_H_
#define INCLUDE_CRYPTO_H_

#include "../BuildConfig.h"
#include "BasicConstants.h"
#include "BasicTypes.h"
#include "md5.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Convert string in hexadecimal format into array of bytes.
 *
 * @param hexStr a pointer to string with hexadecimal format.
 * @param hexStr_size length of hexStr char array.
 * @param out_byteArr a pointer to an BYTE array with allocated memory which size is hexStrSize / 2.
 * @return 0 if success; 1 is fail.
 */
int fromHexStrToByteArr(const char* _hexStr, int _hexStr_size, OUT_ARG BYTE* _out_byteArr);

/**
 * Convert array of bytes into string with hexadecimal format.
 *
 * @param byteArr a pointer to array of bytes.
 * @param byteArr_size length of byteArr array.
 * @param out_strArr a pointer to an char array with allocated memory which size is hexStrSize * 2.
 * @return 0 if success; 1 is fail.
 */
int fromByteArrToHexStr(const BYTE* _byteArr, int _byteArr_size, OUT_ARG char** _out_strArr);

/**
 * Compare two md5 hashes with MD5_BLOCK_SIZE length.
 * @return 0 if hashes are the same.
 */
int cmpHash_md5(const BYTE _hash1[MD5_BLOCK_SIZE], const BYTE _hash2[MD5_BLOCK_SIZE]);

#endif /* INCLUDE_CRYPTO_H_ */
