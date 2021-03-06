/*
 * Config.h
 *
 *  Created on: 8 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

#include "../BuildConfig.h"
#include "BasicTypes.h"
#include "BasicConstants.h"
#include "Common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>

#define SFTU_CONFIG_FIELDNAME_PORT "port"
#define SFTU_CONFIG_FIELDNAME_STORAGE_DIR_PATH "storage-path"
#define SFTU_CONFIG_FIELDNAME_PASSWORD "password"
#define SFTU_CONFIG_FIELDNAME_MAX_PEERS "max-connected-peers"
#define SFTU_CONFIG_FIELDNAME_MAX_IN_FILESIZE "max-input-file-size"

int config_loadFromFile(char* _conf_file_path, OUT_ARG serverConfig_t* _serv_conf_ptr);
void config_free(serverConfig_t* _serv_conf_ptr);

#endif /* INCLUDE_CONFIG_H_ */
