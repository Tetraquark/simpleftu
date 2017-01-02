/*
 * server.c
 *
 *  Created on: 8 окт. 2016 г.
 *      Author: tetraquark
 */

#include "../include/Server.h"

int startTCPListener(int _listenPort, serverMode_t _serverMode){
	int rc = EXIT_SUCCESS;

	struct sockaddr_in tcpSockaddr;
	int socketFd = 0;
	int listenPort = _listenPort;
	int sockCommPipes[2];
	serverSysInfo_t serverThreadInfo;
	pthread_t serverThread = 0;

	if(pipe(sockCommPipes) == -1)
		return EXIT_FAILURE;

	// create TCP listener socket
	rc |= createServTCPSocket(&tcpSockaddr, &socketFd, listenPort);

	serverThreadInfo.inputCommsPipeFd = sockCommPipes[1];
	serverThreadInfo.socketFd = socketFd;
	serverThreadInfo.mode = _serverMode;

	// start server TCP listener pthread
	if( pthread_create(&serverThread, NULL, startListenTCPSocket, (void*) &serverThreadInfo) ){
		// Creating pthread Error

		// close commands pipes
		for(int i = 0; i < 2; i++)
			close(sockCommPipes[i]);

		return EXIT_FAILURE;
	}
	else{
		// if pthread creating was successful
		close(sockCommPipes[1]);

		pthread_join(serverThread, NULL);
	}

	return rc;
}

/**
 * Server TCP listener pthread function.
 * Work with while statement. Run accept() for input clients. Then create
 * new pthreads for all input clients.
 */
void* startListenTCPSocket(void* threadData){
	serverCommands_t currCommAction = LISTEN;

	serverSysInfo_t* serverInfo = (serverSysInfo_t*) threadData;

	while(currCommAction != STOP_SERVER){

		if(currCommAction == LISTEN){

			struct sockaddr_in tcpInputClient_addr;
			int inputClientFd = 0;

			if ((inputClientFd = accept(serverInfo->socketFd, &tcpInputClient_addr, (sizeof(tcpInputClient_addr)))) < 0){
				return EXIT_FAILURE;
			}


		}
	}

}

/**
 * Input client (peer) pthread function.
 * Protocol:
 * After connection client send:
 * 1) client send password: char[MAX_PASS_LEN]
 * 2) client send file info struct: file_info_msg_t
 * 3) client send file
 */
void* startPeerThread(void* threadData){
	serverSysInfo_t* connectInfo = (serverSysInfo_t*) threadData;
	char* servPassword = connectInfo->conf.password;		// server password
	char* servDownloadFolder = connectInfo->conf.saveFilesFolder;
	int inputConnectFd = connectInfo->socketFd;				// input connection FD
	int inputMsgSize = 0;

	// get input password
	char inputPassBuff[MAX_PASS_LEN];
	memset(&inputPassBuff, 0, sizeof(inputPassBuff));
	if((inputMsgSize = recvfrom(inputConnectFd, &inputPassBuff, MAX_PASS_LEN * sizeof(char), 0, NULL, 0)) < 0){
		// Receiving error
		return EXIT_FAILURE;
	}

	// check input password
	if(checkPassword(servPassword, inputPassBuff) == FALSE){
		// if wrong password - close connection and close pthread
		close(inputConnectFd);
		return EXIT_FAILURE;
	}

	inputMsgSize = 0;

	// get file info struct
	file_info_msg_t inputFileInfo;
	size_t inputMsgBuffSize = sizeof(char) * MAX_FILESIZE_CHAR_NUM + sizeof(char) * MAX_FILENAME_LEN;
	char* fileInfoMsgBuff = (char*) malloc(inputMsgBuffSize);
	memset(fileInfoMsgBuff, 0, inputMsgBuffSize);

	if((inputMsgSize = recvfrom(inputConnectFd, fileInfoMsgBuff, inputMsgBuffSize, 0, NULL, 0)) < 0){
		// Receiving error
		return EXIT_FAILURE;
	}
	if(deserialize_FileInfoMsg(&inputFileInfo, fileInfoMsgBuff, ';')){
		// Deserialize error
		return EXIT_FAILURE;
	}

	size_t recvFuleFullPathSize = strlen(servDownloadFolder) * sizeof(char) + strlen(inputFileInfo.fileName) * sizeof(char);
	char* recvFileFullPath = (char*) malloc(recvFuleFullPathSize);
	memset(recvFileFullPath, 0, recvFuleFullPathSize);
	strncpy(recvFileFullPath, servDownloadFolder, strlen(servDownloadFolder));
	strncpy(&recvFileFullPath[strlen(servDownloadFolder)], inputFileInfo.fileName, strlen(inputFileInfo.fileName));

	/*
	FILE* recvFile = fopen(recvFileFullPath, "wb");
	if(recvFile == NULL){
		// file opening error
		return EXIT_FAILURE;
	}
	*/

	// getting file
	file_size_t remainData = inputFileInfo.fileSize;


	// close pthread
	close(inputConnectFd);
	//fclose(recvFile);
	free(fileInfoMsgBuff);
	free(recvFileFullPath);
	return EXIT_SUCCESS;
}


/**
 * Compare two char arrays with passwords.
 * @return TRUE if passwords is same
 */
bool_t checkPassword(const char* servPassStr, const char* inputPassStr){
	int cmp_res = 0;

	cmp_res = strncmp(servPassStr, inputPassStr, MAX_PASS_LEN * sizeof(char));

	if(!cmp_res)
		return TRUE;

	return FALSE;
}

/**
 * Create socket listener for _socket_desc descriptor and bind into _port.
 * @return 0 success; 1 error
 */
int createServTCPSocket(struct sockaddr_in* _tcpsocket_addr, int* _socket_desc, int _port){

	if((*_socket_desc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return EXIT_FAILURE;

    memset(_tcpsocket_addr, 0, sizeof(*_tcpsocket_addr));
    (*_tcpsocket_addr).sin_family      = AF_INET;
    (*_tcpsocket_addr).sin_addr.s_addr = htonl(INADDR_ANY);;
    (*_tcpsocket_addr).sin_port        = htons(_port);

    if( bind(*_socket_desc, _tcpsocket_addr, sizeof(*_tcpsocket_addr)) < 0 )
    	return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

