/*
 * Serializer.h
 *
 *  Created on: 1 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_SERIALIZER_H_
#define INCLUDE_SERIALIZER_H_

#include "../BuildConfig.h"
#include "BasicConstants.h"
#include "BasicTypes.h"
#include "Crypto.h"
#include "Common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

ssize_t serialize_FileInfoMsg(file_info_msg_t _inStruct, const char _delimSymbol, OUT_ARG char** _out_fileInfoMsg);

int deserialize_FileInfoMsg(OUT_ARG file_info_msg_t* _outStruct, char* _msgBuff, const char _delimSymbol);

/**
 * Parse string with format: "255.255.255.255:10888" into parts ip and port.
 *
 * @param addr_str string with format like "255.255.255.255:10888"
 * @param ip_str OUTPUT arg. Pointer to char string for parsed ip address. Default is string with 16 chars size.
 * @param port OUTPUT arg. Pointer to int variable for parsed port.
 */
int parse_ipaddrStrToParts(char* _addr_str, OUT_ARG char** _ip_str, OUT_ARG int* _port);

#endif /* INCLUDE_SERIALIZER_H_ */
