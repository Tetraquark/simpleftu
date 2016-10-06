/*
 * main.c
 *
 *  Created on: 5 окт. 2016 г.
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

#include "../include/BasicTypes.h"
#include "../include/BasicConstants.h"

#define GETOPT_ARGS "dhcf:a:"

int runDaemonMode(){
	int rc = EXIT_SUCCESS;

	return rc;
}

int runClientMode(char* _serverAddr, char* _filePath){
	int rc = EXIT_SUCCESS;

	return rc;
}

int runServerMode(){
	int rc = EXIT_SUCCESS;

	return rc;
}

int main(int argc, char** argv){
	pid_t daemon_pid;
	int option = 0;
	int rc = EXIT_SUCCESS;

	mode_type_t appMode = MODE_NONE;
	char* sendFilePath = NULL;
	char* serverAddr = NULL;

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
					"-c\t : Run in client mode. Sends file -f (path to file) to -a (address) server on default port: %d.\n"
					"-f arg\t : Works only with -c options. Sets the file path on local disk.\n"
					"-a arg\t : Works only with -c options. Sets the server address (IPv4)\n\n", DEFAULT_SERVER_PORT);
			if(sendFilePath != NULL){
				free(sendFilePath);
			}
			if(serverAddr != NULL)
				free(serverAddr);
			return EXIT_SUCCESS;
			break;
		case 'd':
			if(appMode != MODE_NONE && appMode != MODE_DAEMON){
				printf("Error - selected multiple modes. Select only one mode.\n");
				return EXIT_FAILURE;
			}

			appMode = MODE_DAEMON;
			break;
		case 'c':
			if(appMode != MODE_NONE && appMode != MODE_CLIENT){
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
	if(appMode == MODE_CLIENT){
#ifdef DEBUG
		printf("Run Client mode.\n");
#endif

		rc |= runClientMode(serverAddr, sendFilePath);

		// free memory
		if(sendFilePath != NULL){
			free(sendFilePath);
		}
		if(serverAddr != NULL){
			free(serverAddr);
		}

		return rc;
	}
	if(appMode == MODE_SERVER){
#ifdef DEBUG
		printf("Run Server mode.\n");
#endif

		rc |= runServerMode();

		// free memory
		if(sendFilePath != NULL){
			free(sendFilePath);
		}
		if(serverAddr != NULL){
			free(serverAddr);
		}

		return rc;
	}
}

