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

int runDaemon(){
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
					"-h\t : Shows this help-message.\n"
					"-d\t : Run in daemon mode. Loads settings from config and starts listen network port for incoming files.\n"
					"-c\t : Run in client mode. Sends file -f (path to file) to -a (address) server on default port: %d.\n"
					"-f arg\t : Works only with -c options. Sets the file path on local disk.\n"
					"-a arg\t : Works only with -c options. Sets the server address (IPv4)\n\n", DEFAULT_SERVER_PORT);
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
			break;
		case 'f':
			break;
		case 'a':
			break;
		}
		option = getopt(argc, argv, GETOPT_ARGS);
	}

	if(appMode == MODE_CLIENT && (sendFilePath == NULL || serverAddr == NULL)){
		printf("Was selected client mode but forgot set -f or -a arguments! Look -h help message.\n");
		return EXIT_FAILURE;
	}

	daemon_pid = fork();
	if(daemon_pid == -1){
		printf("Can't start daemon: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if(daemon_pid == 0){
		umask(0);
		setsid();
		chdir("/");

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		rc |= runDaemon();

		return rc;
	}
	else{
		printf("Daemon started successful.\n");
		return EXIT_SUCCESS;
	}
}

