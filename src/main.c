/*
 * main.c
 *
 *  Created on: 5 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <execinfo.h>

#include <sys/stat.h>
#include <sys/file.h>

#include "../BuildConfig.h"
#include "../include/BasicTypes.h"
#include "../include/BasicConstants.h"
#include "../include/Common.h"
#include "../include/Server.h"
#include "../include/Client.h"
#include "../include/Config.h"

int runDaemonMode(){
	int _rc = EXIT_SUCCESS;

	return _rc;
}

int runClientMode(char* __serverAddr, int _serverPort, char* _filePath, char* _serverPass){
	int _rc = EXIT_SUCCESS;

#ifdef DEBUG
	_rc = DEBUG_sendTestFile(__serverAddr, _serverPort, _filePath, _serverPass);
	if(_rc)
		logMsg(__func__, __LINE__, INFO, "Unsuccessful file transfer");
	else
		logMsg(__func__, __LINE__, INFO, "Successful file transfer");
#endif

	return _rc;
}

int runServerMode(serverConfig_t* conf){
	int _rc = EXIT_SUCCESS;
	startServTCPListener(conf);
	return _rc;
}

void getOpt_helpMsg(){
	printf("Options and arguments:\n"
			"-h --help\t : Shows this help-message and exit.\n\n"

			"-d --daemon\t : Run in daemon mode. Loads settings from config and starts listen network port as server for incoming files.\n"
			"-s --server\t : Run in server mode. \n"
			"-c --client\t : Run in client (peer) mode. Sends file --file (path to file) to --saddr (address) server.\n\n"

			"-f --file\t : Works only with client (-c) mode. Sets the path of the sent file.\n"
			"-a --saddr\t : Works only with client (-c) mode. Sets the server address (IPv4) in format \"IPv4:Port\"(127.0.0.1:10888)\n"
			"-p --cpass\t : Works only with client (-c) mode. Sets client password.\n\n"

			"--conf\t\t : Works only with -s and -d modes. Sets config file path. Optional arg. Empty arg means load config from default path.\n"
			"--port\t\t : Works only with -s and -d modes. Sets server TCP port.\n"
			"--spass\t\t : Works only with -s and -d modes. Sets password on server. Maximum %d symbols.\n"
			"--storage\t : Works only with -s and -d modes. Sets server storage dir path.\n"

			, MAX_PASS_LEN);
}

int main(int argc, char** argv){
	int _rc = EXIT_SUCCESS;

	mode_type_t appMode = MODE_NONE;
	char* _sendFilePath = NULL;
	char* _serverAddr = NULL;
	char* _clientPass = NULL;
	int serverPort = 0;

	bool_t isLoadFromCfg = FALSE;
	char* _confFilePath = NULL;
	serverConfig_t servConf_st;
	servConf_st.port = -1;
	servConf_st.password = NULL;
	servConf_st.storageFolderPath = NULL;

	pid_t daemon_pid;

	int option = 0;
	int option_index = 0;
	const char* short_options = "hdscp:f:a:";
	const struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},

		// Possible app modes:
		{"daemon", no_argument, NULL, 'd'},
		{"server", no_argument, NULL, 's'},
		{"client", no_argument, NULL, 'c'},

		// Args in Client mode:
		{"file", required_argument, NULL, 'f'},
		{"saddr", required_argument, NULL, 'a'},
		{"saddr", required_argument, NULL, 'p'},

		// Args in Server mode:
		{"conf", optional_argument, NULL, 'q'},
		{"port", required_argument, NULL, 't'},
		{"spass", required_argument, NULL, 'w'},
		{"storage", required_argument, NULL, 'b'},

		{NULL,0,NULL,0}
	};

	/*
	 * Parse input program options.
	 */
	option = getopt_long(argc, argv, short_options, long_options, &option_index);
	if(option < 0){
		printf("Error: mode not selected! Look -h help message. Exit.\n");
		_rc = EXIT_FAILURE;
		goto __exit_1;
	}

	while(option != -1){
		switch(option){
		case 'h':
			getOpt_helpMsg();

			_rc = EXIT_SUCCESS;
			goto __exit_1;
			break;
		case 's':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode. Exit.\n");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}

			appMode = MODE_SERVER;
			break;
		case 'd':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode. Exit.\n");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}

			appMode = MODE_DAEMON;
			break;
		case 'c':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode. Exit.\n");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}

			appMode = MODE_CLIENT;
			break;
		case 'f':
			if(appMode != MODE_CLIENT){
				printf("Error: -f option work only in client mode. Exit.\n");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
			_sendFilePath = (char*) malloc(MAX_FULL_FILE_PATH_LEN + 1 * sizeof(char));
			memset(_sendFilePath, '\0', MAX_FULL_FILE_PATH_LEN + 1 * sizeof(char));
			strncpy(_sendFilePath, optarg, strlen(optarg) * sizeof(char));
			break;
		case 'a':
			if(appMode != MODE_CLIENT){
				printf("Error: -a option work only in client mode. Exit.\n");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
			_serverAddr = (char*) malloc(IPADDR_STR_LEN + 1 * sizeof(char));
			memset(_serverAddr, '\0', IPADDR_STR_LEN + 1 * sizeof(char));
			if(parse_ipaddrStrToParts(optarg, &_serverAddr, &serverPort)){
				logMsg(__func__, __LINE__, ERROR,
						"Parsing server address error. Correct input string format: \"IP:PORT\"(127.0.0.1:10888). Exit.");

				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
			break;
		case 'p':	// --cpass option
			if(appMode != MODE_CLIENT){
				printf("Error: -p option work only in client mode. Exit.\n");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
			_clientPass = (char*) malloc(MAX_PASS_LEN + 1 * sizeof(char));
			memset(_clientPass, '\0', MAX_PASS_LEN + 1 * sizeof(char));
			strncpy(_clientPass, optarg, strlen(optarg) * sizeof(char));
			break;
		case 'q':	// --conf option
			if(appMode != MODE_SERVER && appMode != MODE_DAEMON){
				logMsg(__func__, __LINE__, ERROR, "--conf option work only in server or daemon mode. Exit.");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
			if(optarg != NULL){
				// load conf from optarg path
				_confFilePath = (char*) malloc(MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));
				memset(_confFilePath, '\0', MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));
				strncpy(_confFilePath, optarg, strlen(optarg) * sizeof(char));
			}
			else{
				// load conf from default path
				_confFilePath = (char*) malloc(strlen(DEFAULT_CONFFILE_NAME) + 1 * sizeof(char));
				memset(_confFilePath, '\0', strlen(DEFAULT_CONFFILE_NAME) + 1 * sizeof(char));
				strncpy(_confFilePath, DEFAULT_CONFFILE_NAME, strlen(DEFAULT_CONFFILE_NAME) * sizeof(char));
			}
			isLoadFromCfg = TRUE;
			break;
		case 't':	// --port option
			if(appMode != MODE_SERVER && appMode != MODE_DAEMON){
				logMsg(__func__, __LINE__, ERROR, "--port option work only in server or daemon mode. Exit.");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
			servConf_st.port = atoi(optarg);
			break;
		case 'w':	// --pass option
			if(appMode != MODE_SERVER && appMode != MODE_DAEMON){
				logMsg(__func__, __LINE__, ERROR, "--spass option work only in server or daemon mode. Exit.");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
			servConf_st.password = (char*) malloc(MAX_PASS_LEN + 1 * sizeof(char));
			memset(servConf_st.password, '\0', MAX_PASS_LEN + 1 * sizeof(char));
			strncpy(servConf_st.password, optarg, strlen(optarg) * sizeof(char));
			break;
		case 'b':	// --storage option
			if(appMode != MODE_SERVER && appMode != MODE_DAEMON){
				logMsg(__func__, __LINE__, ERROR, "--storage option work only in server or daemon mode. Exit.");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
			servConf_st.storageFolderPath = (char*) malloc(MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));
			memset(servConf_st.password, '\0', MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));
			strncpy(servConf_st.password, optarg, strlen(optarg) * sizeof(char));
			break;
		}
		option = getopt_long(argc, argv, short_options, long_options, &option_index);
		option_index = -1;
	}

	if(appMode == MODE_NONE){
		logMsg(__func__, __LINE__, ERROR, "Forgot select program mode! Look -h help message. Exit.");

		_rc = EXIT_FAILURE;
		goto __exit_1;
	}

	if(appMode == MODE_SERVER || appMode == MODE_DAEMON){
		if(isLoadFromCfg == TRUE){
			// Load server settings from config.
			config_loadFromFile(_confFilePath, &servConf_st);
		}
		else{
			if(servConf_st.password == NULL || servConf_st.storageFolderPath == NULL
					|| servConf_st.port == -1)
			{
				logMsg(__func__, __LINE__, ERROR, "Wrong server config. Restart program with correct params. Exit.");
				_rc = EXIT_FAILURE;
				goto __exit_1;
			}
		}
	}

	if(appMode == MODE_CLIENT && (_sendFilePath == NULL || _serverAddr == NULL)){
		printf("Was selected client mode but forgot set -f or -a argument! Look -h help message.\n");
		_rc = EXIT_FAILURE;
		goto __exit_1;
	}

	if(appMode == MODE_DAEMON){
		daemon_pid = fork();
		if(daemon_pid == -1){
			logMsg(__func__, __LINE__, ERROR, "Can't start daemon: %s", strerror(errno));
			_rc = EXIT_FAILURE;
			goto __exit_1;
		}
		else if(daemon_pid == 0){
			printf("Daemon started successful.\n");
			umask(0);
			setsid();
			chdir("/");

			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			_rc |= runDaemonMode();

		}
		else{
#ifdef DEBUG
			printf("Close current process.\n");
#endif
		}
	}
	else if(appMode == MODE_CLIENT){
		logMsg(__func__, __LINE__, INFO, "Run Client mode. Connect to: %s:%d; Send file: %s", _serverAddr, serverPort, _sendFilePath);

		_rc |= runClientMode(_serverAddr, DEFAULT_SERVER_PORT, _sendFilePath, _clientPass);

	}
	else if(appMode == MODE_SERVER){
		logMsg(__func__, __LINE__, INFO, "Run Server mode: ");

		_rc |= runServerMode(&servConf_st);

	}

	__exit_1:
	if(_sendFilePath != NULL)
		free(_sendFilePath);
	if(_serverAddr != NULL)
		free(_serverAddr);
	if(_clientPass != NULL)
		free(_clientPass);
	if(_confFilePath != NULL)
		free(_confFilePath);
	config_free(&servConf_st);

	logMsg(__func__, __LINE__, INFO, "Stop program with result code: %d", _rc);
	return _rc;
}

