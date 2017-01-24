/*
 * Config.c
 *
 *  Created on: 8 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Config.h"

int config_loadFromFile(char* _conf_file_path, OUT_ARG serverConfig_t* _serv_conf_ptr){
	if(_serv_conf_ptr == NULL)
		return EXIT_FAILURE;

	int rc = EXIT_SUCCESS;
	config_t cfg;

	config_init(&cfg);

	/*
	 * Read config from file into libconfig struct.
	 */
    if (!config_read_file(&cfg, _conf_file_path)){
    	logMsg(__func__, __LINE__, ERROR, "%s : <%s:%d>", config_error_text(&cfg), config_error_file(&cfg), config_error_line(&cfg));
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    /*
     * Get "port" field from cfg.
     */
    int port = 0;
    rc = config_lookup_int(&cfg, SFTU_CONFIG_FIELDNAME_PORT, &port);
    if(rc == CONFIG_FALSE){
    	logMsg(__func__, __LINE__, ERROR, "Error parsing config field %s", SFTU_CONFIG_FIELDNAME_PORT);
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }
    _serv_conf_ptr->port = port;

    /*
     * Get "storage-path" field from cfg.
     */
    char* storage_folder;
    rc = config_lookup_string(&cfg, SFTU_CONFIG_FIELDNAME_STORAGE_DIR_PATH, &storage_folder);
    if(rc == CONFIG_FALSE){
    	logMsg(__func__, __LINE__, ERROR, "Error parsing config field %s", SFTU_CONFIG_FIELDNAME_STORAGE_DIR_PATH);
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }
    _serv_conf_ptr->storageFolderPath = (char*) malloc(MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));		// will free in config_free()
    memset(_serv_conf_ptr->storageFolderPath, '\0', MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));
    strncpy(_serv_conf_ptr->storageFolderPath, storage_folder, strlen(storage_folder) * sizeof(char));

    /*
     * Get "password" field from cfg.
     */
    char* password;
    rc = config_lookup_string(&cfg, SFTU_CONFIG_FIELDNAME_PASSWORD, &password);
    if(rc == CONFIG_FALSE){
    	logMsg(__func__, __LINE__, ERROR, "Error parsing config field %s", SFTU_CONFIG_FIELDNAME_PASSWORD);
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }
    _serv_conf_ptr->password = (char*) malloc(MAX_PASS_LEN + 1 * sizeof(char));		// will free in config_free()
    memset(_serv_conf_ptr->password, '\0', MAX_PASS_LEN + 1 * sizeof(char));
    strncpy(_serv_conf_ptr->password, password, strlen(password) * sizeof(char));

    /*
     * Get "max-connected-peers" field from cfg.
     */
    // TODO:

    /*
     * Get "max-input-file-size" field from cfg.
     */
    // TODO:

    config_destroy(&cfg);
    return EXIT_SUCCESS;
}

void config_free(serverConfig_t* _serv_conf_ptr){
	if(_serv_conf_ptr == NULL)
		return;

	if(_serv_conf_ptr->password)
		free(_serv_conf_ptr->password);

	if(_serv_conf_ptr->storageFolderPath)
		free(_serv_conf_ptr->storageFolderPath);
}
