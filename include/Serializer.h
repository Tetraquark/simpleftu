/*
 * Serializer.h
 *
 *  Created on: 1 янв. 2017 г.
 *      Author: tetraquark
 */

#ifndef INCLUDE_SERIALIZER_H_
#define INCLUDE_SERIALIZER_H_

#include <string.h>
#include <stdlib.h>

#include "BasicConstants.h"
#include "BasicTypes.h"
#include "Crypto.h"

char* serialize_FileInfoMsg(file_info_msg_t inStruct, const char delimSymbol);

int deserialize_FileInfoMsg(const file_info_msg_t* outStruct, char* msgBuff, const char delimSymbol);

#endif /* INCLUDE_SERIALIZER_H_ */
