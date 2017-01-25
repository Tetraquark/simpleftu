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

#include <sys/stat.h>
#include <sys/file.h>

#ifdef __linux__
#include <wait.h>
#include <execinfo.h>
#endif

#include "../BuildConfig.h"
#include "../include/BasicTypes.h"
#include "../include/BasicConstants.h"
#include "../include/Common.h"
#include "../include/Server.h"
#include "../include/Client.h"
#include "../include/Config.h"

int runDaemonMode(){
	int rc = EXIT_SUCCESS;

	return rc;
}

int runClientMode(char* _serverAddr, int _serverPort, char* _filePath, char* _serverPass){
	int rc = EXIT_SUCCESS;

	rc = startClient(_serverAddr, _serverPort, _filePath, _serverPass);

	return rc;
}

int runServerMode(serverConfig_t* _conf){
	int rc = EXIT_SUCCESS;
	startServTCPListener(_conf);
	return rc;
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
	int rc = EXIT_SUCCESS;

	mode_type_t appMode = MODE_NONE;
	char* sendFilePath = NULL;
	char* serverAddr = NULL;
	char* clientPass = NULL;
	int serverPort = 0;

	bool_t isLoadFromCfg = B_FALSE;
	char* confFilePath = NULL;
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
		rc = EXIT_FAILURE;
		goto __exit_1;
	}

	while(option != -1){
		switch(option){
		case 'h':
			getOpt_helpMsg();

			rc = EXIT_SUCCESS;
			goto __exit_1;
			break;
		case 's':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode. Exit.\n");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}

			appMode = MODE_SERVER;
			break;
		case 'd':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode. Exit.\n");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}

			appMode = MODE_DAEMON;
			break;
		case 'c':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode. Exit.\n");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}

			appMode = MODE_CLIENT;
			break;
		case 'f':
			if(appMode != MODE_CLIENT){
				printf("Error: -f option work only in client mode. Exit.\n");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}
			sendFilePath = (char*) malloc(MAX_FULL_FILE_PATH_LEN + 1 * sizeof(char));
			memset(sendFilePath, '\0', MAX_FULL_FILE_PATH_LEN + 1 * sizeof(char));
			strncpy(sendFilePath, optarg, strlen(optarg) * sizeof(char));
			break;
		case 'a':
			if(appMode != MODE_CLIENT){
				printf("Error: -a option work only in client mode. Exit.\n");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}
			serverAddr = (char*) malloc(IPADDR_STR_LEN + 1 * sizeof(char));
			memset(serverAddr, '\0', IPADDR_STR_LEN + 1 * sizeof(char));
			if(parse_ipaddrStrToParts(optarg, &serverAddr, &serverPort)){
				logMsg(__func__, __LINE__, LOG_ERROR,
						"Parsing server address error. Correct input string format: \"IP:PORT\"(127.0.0.1:10888). Exit.");

				rc = EXIT_FAILURE;
				goto __exit_1;
			}
			break;
		case 'p':	// --cpass option
			if(appMode != MODE_CLIENT){
				printf("Error: -p option work only in client mode. Exit.\n");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}
			clientPass = (char*) malloc(MAX_PASS_LEN + 1 * sizeof(char));
			memset(clientPass, '\0', MAX_PASS_LEN + 1 * sizeof(char));
			strncpy(clientPass, optarg, strlen(optarg) * sizeof(char));
			break;
		case 'q':	// --conf option
			if(appMode != MODE_SERVER && appMode != MODE_DAEMON){
				logMsg(__func__, __LINE__, LOG_ERROR, "--conf option work only in server or daemon mode. Exit.");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}
			if(optarg != NULL){
				// load conf from optarg path
				confFilePath = (char*) malloc(MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));
				memset(confFilePath, '\0', MAX_STORAGEDIR_PATH_LEN + 1 * sizeof(char));
				strncpy(confFilePath, optarg, strlen(optarg) * sizeof(char));
			}
			else{
				// load conf from default path
				confFilePath = (char*) malloc(strlen(DEFAULT_CONFFILE_NAME) + 1 * sizeof(char));
				memset(confFilePath, '\0', strlen(DEFAULT_CONFFILE_NAME) + 1 * sizeof(char));
				strncpy(confFilePath, DEFAULT_CONFFILE_NAME, strlen(DEFAULT_CONFFILE_NAME) * sizeof(char));
			}
			isLoadFromCfg = B_TRUE;
			break;
		case 't':	// --port option
			if(appMode != MODE_SERVER && appMode != MODE_DAEMON){
				logMsg(__func__, __LINE__, LOG_ERROR, "--port option work only in server or daemon mode. Exit.");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}
			servConf_st.port = atoi(optarg);
			break;
		case 'w':	// --pass option
			if(appMode != MODE_SERVER && appMode != MODE_DAEMON){
				logMsg(__func__, __LINE__, LOG_ERROR, "--spass option work only in server or daemon mode. Exit.");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}
			servConf_st.password = (char*) malloc(MAX_PASS_LEN + 1 * sizeof(char));
			memset(servConf_st.password, '\0', MAX_PASS_LEN + 1 * sizeof(char));
			strncpy(servConf_st.password, optarg, strlen(optarg) * sizeof(char));
			break;
		case 'b':	// --storage option
			if(appMode != MODE_SERVER && appMode != MODE_DAEMON){
				logMsg(__func__, __LINE__, LOG_ERROR, "--storage option work only in server or daemon mode. Exit.");
				rc = EXIT_FAILURE;
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
		logMsg(__func__, __LINE__, LOG_ERROR, "Forgot select program mode! Look -h help message. Exit.");

		rc = EXIT_FAILURE;
		goto __exit_1;
	}

	if(appMode == MODE_SERVER || appMode == MODE_DAEMON){
		if(isLoadFromCfg == B_TRUE){
			// Load server settings from config.
			config_loadFromFile(confFilePath, &servConf_st);
		}
		else{
			if(servConf_st.password == NULL || servConf_st.storageFolderPath == NULL
					|| servConf_st.port == -1)
			{
				logMsg(__func__, __LINE__, LOG_ERROR, "Wrong server config. Restart program with correct params. Exit.");
				rc = EXIT_FAILURE;
				goto __exit_1;
			}
		}
	}

	if(appMode == MODE_CLIENT && (sendFilePath == NULL || serverAddr == NULL)){
		printf("Was selected client mode but forgot set -f or -a argument! Look -h help message.\n");
		rc = EXIT_FAILURE;
		goto __exit_1;
	}

#ifdef __linux__
	if(appMode == MODE_DAEMON){
		daemon_pid = fork();
		if(daemon_pid == -1){
			logMsg(__func__, __LINE__, LOG_ERROR, "Can't start daemon: %s", strerror(errno));
			rc = EXIT_FAILURE;
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

			rc |= runDaemonMode();

		}
		else{
#ifdef DEBUG
			printf("Close current process.\n");
#endif
		}
	}
#endif
	else if(appMode == MODE_CLIENT){
		logMsg(__func__, __LINE__, LOG_INFO, "Run Client mode. Connect to: %s:%d; Send file: %s", serverAddr, serverPort, sendFilePath);

		rc |= runClientMode(serverAddr, serverPort, sendFilePath, clientPass);

		if(rc)
			logMsg(__func__, __LINE__, LOG_INFO, "Unsuccessful file transfer.");
		else
			logMsg(__func__, __LINE__, LOG_INFO, "Successful file transfer.");
	}
	else if(appMode == MODE_SERVER){
		logMsg(__func__, __LINE__, LOG_INFO, "Run Server mode: ");

		rc |= runServerMode(&servConf_st);

	}

	__exit_1:
	if(sendFilePath != NULL)
		free(sendFilePath);
	if(serverAddr != NULL)
		free(serverAddr);
	if(clientPass != NULL)
		free(clientPass);
	if(confFilePath != NULL)
		free(confFilePath);
	config_free(&servConf_st);

	logMsg(__func__, __LINE__, LOG_INFO, "Stop program with result code: %d", rc);
	return rc;
}

