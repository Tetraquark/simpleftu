/*
 * Config.c
 *
 *  Created on: 8 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Config.h"

int config_loadFromFile(char* conf_file_path, OUT_ARG serverConfig_t* serv_conf_ptr){
	if(serv_conf_ptr == NULL)
		return EXIT_FAILURE;

	int _rc = EXIT_SUCCESS;
	config_t _cfg;

	config_init(&_cfg);

	/*
	 * Read config from file into libconfig struct.
	 */
    if (!config_read_file(&_cfg, conf_file_path)){
    	logMsg(__func__, __LINE__, ERROR, "%s : <%s:%d>", config_error_text(&_cfg), config_error_file(&_cfg), config_error_line(&_cfg));
        config_destroy(&_cfg);
        return EXIT_FAILURE;
    }

    /*
     * Get "port" field from cfg.
     */
    int _port = 0;
    _rc = config_lookup_int(&_cfg, SFTU_CONFIG_FIELDNAME_PORT, &_port);
    if(_rc == CONFIG_FALSE){
    	logMsg(__func__, __LINE__, ERROR, "Error parsing config field %s", SFTU_CONFIG_FIELDNAME_PORT);
        config_destroy(&_cfg);
        return EXIT_FAILURE;
    }
    serv_conf_ptr->port = _port;

    /*
     * Get "storage-path" field from cfg.
     */
    char* _storage_folder;
    _rc = config_lookup_string(&_cfg, SFTU_CONFIG_FIELDNAME_STORAGE_DIR_PATH, &_storage_folder);
    if(_rc == CONFIG_FALSE){
    	logMsg(__func__, __LINE__, ERROR, "Error parsing config field %s", SFTU_CONFIG_FIELDNAME_STORAGE_DIR_PATH);
        config_destroy(&_cfg);
        return EXIT_FAILURE;
    }
    serv_conf_ptr->storageFolderPath = (char*) malloc(MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));		// will free in config_free()
    memset(serv_conf_ptr->storageFolderPath, '\0', MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));
    strncpy(serv_conf_ptr->storageFolderPath, _storage_folder, strlen(_storage_folder) * sizeof(char));

    /*
     * Get "password" field from cfg.
     */
    char* _password;
    _rc = config_lookup_string(&_cfg, SFTU_CONFIG_FIELDNAME_PASSWORD, &_password);
    if(_rc == CONFIG_FALSE){
    	logMsg(__func__, __LINE__, ERROR, "Error parsing config field %s", SFTU_CONFIG_FIELDNAME_PASSWORD);
        config_destroy(&_cfg);
        return EXIT_FAILURE;
    }
    serv_conf_ptr->password = (char*) malloc(MAX_PASS_LEN + 1 * sizeof(char));		// will free in config_free()
    memset(serv_conf_ptr->password, '\0', MAX_PASS_LEN + 1 * sizeof(char));
    strncpy(serv_conf_ptr->password, _password, strlen(_password) * sizeof(char));

    /*
     * Get "max-connected-peers" field from cfg.
     */
    // TODO:

    /*
     * Get "max-input-file-size" field from cfg.
     */
    // TODO:

    config_destroy(&_cfg);
    return EXIT_SUCCESS;
}

void config_free(serverConfig_t* serv_conf_ptr){
	if(serv_conf_ptr == NULL)
		return;

	if(serv_conf_ptr->password)
		free(serv_conf_ptr->password);

	if(serv_conf_ptr->storageFolderPath)
		free(serv_conf_ptr->storageFolderPath);
}
