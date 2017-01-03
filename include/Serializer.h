/*
 * Serializer.h
 *
 *  Created on: 1 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_SERIALIZER_H_
#define INCLUDE_SERIALIZER_H_

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "BasicConstants.h"
#include "BasicTypes.h"
#include "Crypto.h"
#include "Common.h"

ssize_t serialize_FileInfoMsg(file_info_msg_t inStruct, const char delimSymbol, char** out_fileInfoMsg);

int deserialize_FileInfoMsg(file_info_msg_t* outStruct, char* msgBuff, const char delimSymbol);

#endif /* INCLUDE_SERIALIZER_H_ */
