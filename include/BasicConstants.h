/*
 * BasicConstants.h
 *
 *  Created on: 6 окт. 2016 г.
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_BASICCONSTANTS_H_
#define INCLUDE_BASICCONSTANTS_H_

#include "../BuildConfig.h"

#define DEFAULT_SERVER_PORT 10888

/*
 * MAX_FILE_SIZE
 * Maximum sending file size;
 */
#ifdef DEBUG
// 4 Gb - long long int
#define MAX_FILE_SIZE 4294967296
#elif
#define MAX_FILE_SIZE 4294967296
#endif

/*
 * MAX_PASS_LEN
 * Maximum password length in char symbols;
 */
#define MAX_PASS_LEN 8
#define MAX_FILENAME_LEN 64

#endif /* INCLUDE_BASICCONSTANTS_H_ */
