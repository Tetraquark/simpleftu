/*
 * Config.h
 *
 *  Created on: 8 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>

#include "../BuildConfig.h"
#include "BasicTypes.h"
#include "BasicConstants.h"
#include "Common.h"

#define SFTU_CONFIG_FIELDNAME_PORT "port"
#define SFTU_CONFIG_FIELDNAME_STORAGE_DIR_PATH "storage-path"
#define SFTU_CONFIG_FIELDNAME_PASSWORD "password"
#define SFTU_CONFIG_FIELDNAME_MAX_PEERS "max-connected-peers"
#define SFTU_CONFIG_FIELDNAME_MAX_IN_FILESIZE "max-input-file-size"

void config_free(serverConfig_t* serv_conf_ptr);
int config_loadFromFile(char* conf_file_path, OUT_ARG serverConfig_t* serv_conf_ptr);

#endif /* INCLUDE_CONFIG_H_ */
