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

#define GETOPT_ARGS "dshpf:a:"

int runDaemonMode(){
	int rc = EXIT_SUCCESS;

	return rc;
}

int runClientMode(char* _serverAddr, int _serverPort, char* _filePath){
	int rc = EXIT_SUCCESS;

#ifdef DEBUG
	rc = DEBUG_sendTestFile(_serverAddr, _serverPort, _filePath);
	if(rc)
		logMsg(__func__, __LINE__, INFO, "Unsuccessful file transfer");
	else
		logMsg(__func__, __LINE__, INFO, "Successful file transfer");
#endif

	return rc;
}

int runServerMode(serverConfig_t conf){
	int rc = EXIT_SUCCESS;
	startServTCPListener(conf);
	return rc;
}

int main(int argc, char** argv){
	pid_t daemon_pid;
	int option = 0;
	int rc = EXIT_SUCCESS;

	mode_type_t appMode = MODE_NONE;
	char* sendFilePath = NULL;
	char* serverAddr = NULL;

	serverConfig_t servConfStrct;

	option = getopt(argc, argv, GETOPT_ARGS);

	if(option <= 0){
		printf("Error: mode not selected! Look -h help message.\n");
		return EXIT_FAILURE;
	}

	while(option != -1){
		switch(option){
		case 'h':
			printf("Options and arguments:\n"
					"-h\t : Shows this help-message and exit.\n"
					"-d\t : Run in daemon mode. Loads settings from config and starts listen network port as server for incoming files.\n"
					"-s\t : Run in server mode. \n"
					"-c\t : Works only with -s mode. Sets config file path. \n"
					"-p\t : Run in client (peer) mode. Sends file -f (path to file) to -a (address) server on default port: %d.\n"
					"-f arg\t : Works only with -p mode. Sets the file path on local disk.\n"
					"-a arg\t : Works only with -p mode. Sets the server address (IPv4)\n\n", DEFAULT_SERVER_PORT);
			if(sendFilePath != NULL){
				free(sendFilePath);
			}
			if(serverAddr != NULL)
				free(serverAddr);
			return EXIT_SUCCESS;
			break;
		case 's':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode.\n");
				return EXIT_FAILURE;
			}

#ifdef DEBUG
			strncpy(servConfStrct.password, DEBUG_PASSWORD, MAX_PASS_LEN);
			//servConfStrct.password = DEBUG_PASSWORD;
			servConfStrct.saveFilesFolder = DEBUG_SERVER_FILES_STORAGE_PATH;
			servConfStrct.port = DEBUG_SERVER_PORT;
			servConfStrct.mode = MULTI_CLIENT;
#endif

			appMode = MODE_SERVER;
			break;
		case 'd':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode.\n");
				return EXIT_FAILURE;
			}

			appMode = MODE_DAEMON;
			break;
		case 'p':
			if(appMode != MODE_NONE){
				printf("Error - selected multiple modes. Select only one mode.\n");
				return EXIT_FAILURE;
			}

			appMode = MODE_CLIENT;
			break;
		case 'f':
			sendFilePath = (char*) malloc(strlen(optarg) * sizeof(char));
			strncpy(sendFilePath, optarg, strlen(optarg) * sizeof(char));
			break;
		case 'a':
			serverAddr = (char*) malloc(strlen(optarg) * sizeof(char));
			strncpy(serverAddr, optarg, strlen(optarg) * sizeof(char));
			break;
		}
		option = getopt(argc, argv, GETOPT_ARGS);
	}

	if(appMode == MODE_NONE){
		printf("Forgot select program mode! Look -h help message.\n");

		if(sendFilePath != NULL)
			free(sendFilePath);
		if(serverAddr != NULL)
			free(serverAddr);

		return EXIT_FAILURE;
	}

	if(appMode == MODE_CLIENT && (sendFilePath == NULL || serverAddr == NULL)){
		printf("Was selected client mode but forgot set -f or -a argument! Look -h help message.\n");
		return EXIT_FAILURE;
	}

	if(appMode == MODE_DAEMON){
#ifdef DEBUG
		printf("Run Daemon mode.\n");
#endif
		daemon_pid = fork();
		if(daemon_pid == -1){
			printf("Can't start daemon: %s\n", strerror(errno));

			if(sendFilePath != NULL)
				free(sendFilePath);
			if(serverAddr != NULL)
				free(serverAddr);

			return EXIT_FAILURE;
		}

		if(daemon_pid == 0){
			printf("Daemon started successful.\n");
			umask(0);
			setsid();
			chdir("/");

			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			rc |= runDaemonMode();

			return rc;
		}
		else{
#ifdef DEBUG
			printf("Close current process.\n");
#endif
			// free memory
			if(sendFilePath != NULL){
				free(sendFilePath);
			}
			if(serverAddr != NULL){
				free(serverAddr);
			}

			return EXIT_SUCCESS;
		}
	}
	else if(appMode == MODE_CLIENT){
		logMsg(__func__, __LINE__, INFO, "Run Client mode. Start runClientMode().");
#ifdef DEBUG
		rc |= runClientMode(DEBUG_SERVER_IP, DEFAULT_SERVER_PORT, sendFilePath);
#else
		rc |= runClientMode(serverAddr, DEFAULT_SERVER_PORT, sendFilePath);
#endif
		// free memory
		if(sendFilePath != NULL){
			free(sendFilePath);
		}
		if(serverAddr != NULL){
			free(serverAddr);
		}

	}
	else if(appMode == MODE_SERVER){
		logMsg(__func__, __LINE__, INFO, "Run Server mode. Start runServerMode().");

		rc |= runServerMode(servConfStrct);

		logMsg(__func__, __LINE__, INFO, "RC of function runServerMode(): %d", rc);

		// free memory
		if(sendFilePath != NULL){
			free(sendFilePath);
		}
		if(serverAddr != NULL){
			free(serverAddr);
		}

	}

	logMsg(__func__, __LINE__, INFO, "Stop program with rc: %d", rc);
	return rc;
}

