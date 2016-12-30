/*
 * BasicTypes.h
 *
 *  Created on: 6 окт. 2016 г.
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_BASICTYPES_H_
#define INCLUDE_BASICTYPES_H_

#include "../BuildConfig.h"
#include "../include/BasicConstants.h"

typedef enum{
	FALSE = 0,
	TRUE
} __bool;
typedef __bool bool_t;

typedef enum{
	MODE_NONE,
	MODE_DAEMON,
	MODE_SERVER,
	MODE_CLIENT,
} __mode_type;
typedef __mode_type mode_type_t;

typedef long long int file_size_t;

typedef struct{
	file_size_t fileSize;
	char fileName[MAX_FILENAME_LEN];
} __file_info_msg;
typedef __file_info_msg file_info_msg_t;



#endif /* INCLUDE_BASICTYPES_H_ */
