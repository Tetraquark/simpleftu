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

ssize_t serialize_FileInfoMsg(file_info_msg_t inStruct, const char delimSymbol, OUT_ARG char** out_fileInfoMsg);

int deserialize_FileInfoMsg(OUT_ARG file_info_msg_t* outStruct, char* msgBuff, const char delimSymbol);

/**
 * Parse string with format: "255.255.255.255:10888" into parts ip and port.
 *
 * @param addr_str string with format like "255.255.255.255:10888"
 * @param ip_str OUTPUT arg. Pointer to char string for parsed ip address. Default is string with 16 chars size.
 * @param port OUTPUT arg. Pointer to int variable for parsed port.
 */
int parse_ipaddrStrToParts(char* addr_str, OUT_ARG char** ip_str, OUT_ARG int* port);

#endif /* INCLUDE_SERIALIZER_H_ */
