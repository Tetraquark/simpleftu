/*
 * Crypto.h
 *
 *  Created on: 2 янв. 2017 г.
 *      Author: tetraquark
 */

#ifndef INCLUDE_CRYPTO_H_
#define INCLUDE_CRYPTO_H_

#include <stdlib.h>

#include "../BuildConfig.h"
#include "BasicConstants.h"
#include "BasicTypes.h"
#include "md5.h"

/**
 * Convert string in hexadecimal format into array of bytes.
 *
 * @param hexStr a pointer to string with hexadecimal format.
 * @param hexStrSize length of hexStr char array.
 * @param out_byteArr a pointer to an BYTE array with allocated memory which size is hexStrSize / 2.
 * @return 0 if success; 1 is fail.
 */
int fromHexStrToByteArr(const char* hexStr, int hexStrSize, BYTE* out_byteArr);

/**
 * Convert array of bytes into string with hexadecimal format.
 *
 * @param byteArr a pointer to array of bytes.
 * @param byteArrSize length of byteArr array.
 * @param out_strArr a pointer to an char array with allocated memory which size is hexStrSize * 2.
 * @return 0 if success; 1 is fail.
 */
int fromByteArrToHexStr(const BYTE* byteArr, int byteArrSize, char* out_strArr);

/**
 * Compare two md5 hashes with MD5_BLOCK_SIZE length.
 * @return 0 if hashes are the same.
 */
int md5HashCmp(const BYTE* hash1, const BYTE hash2);

#endif /* INCLUDE_CRYPTO_H_ */
