/*
 * BasicConstants.h
 *
 *  Created on: 6 окт. 2016 г.
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_BASICCONSTANTS_H_
#define INCLUDE_BASICCONSTANTS_H_

#include <limits.h>

#include "../BuildConfig.h"

#define DEFAULT_SERVER_PORT 10888

/*
 * MAX_FILE_SIZE
 * Maximum sending file size;
 */
#ifdef DEBUG
// 32 Gb
#define MAX_FILE_SIZE 34359738368
#define MAX_FILESIZE_CHAR_NUM 11
#else
#define MAX_FILE_SIZE 34359738368
#define MAX_FILESIZE_CHAR_NUM 11
#endif

/*
 * MAX_PASS_LEN
 * Maximum password length in char symbols;
 */
#define MAX_PASS_LEN 8
#define MAX_FILENAME_LEN 128


#endif /* INCLUDE_BASICCONSTANTS_H_ */
