/*
 * BasicTypes.h
 *
 *  Created on: 6 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_BASICTYPES_H_
#define INCLUDE_BASICTYPES_H_

#include <stdint.h>

#include "../BuildConfig.h"
#include "md5.h"
#include "BasicConstants.h"

typedef int64_t file_size_t;

typedef enum{
	FALSE = 0,
	TRUE
} __bool;
typedef __bool bool_t;

typedef enum{
	INFO = 0,
	WARNING,
	ERROR
} __log_msg_type;
typedef __log_msg_type log_msg_type_t;

typedef enum{
	SUCCESSFUL = 0,
	FAIL
} __netmsg_sending_res;
typedef __netmsg_sending_res netmsg_sending_res_t;

typedef enum{
	MODE_NONE,
	MODE_DAEMON,
	MODE_SERVER,
	MODE_CLIENT,
} __mode_type;
typedef __mode_type mode_type_t;

typedef struct{
	file_size_t fileSize;					// in bytes
	char fileName[MAX_FILENAME_LEN];
	//BYTE fileHash_md5[MD5_BLOCK_SIZE];
} __file_info_msg;
typedef __file_info_msg file_info_msg_t;

#endif /* INCLUDE_BASICTYPES_H_ */
