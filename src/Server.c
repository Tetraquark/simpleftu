/*
 * server.c
 *
 *  Created on: 8 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Server.h"

int startServTCPListener(serverConfig_t* serverConf_ptr){
	logMsg(__func__, __LINE__, INFO, "Start server TCP Listener.");

	struct sockaddr_in tcpSockaddr;
	int socketFd = 0;
	int listenPort = serverConf_ptr->port;
	int sockCommPipes[2];
	serverSysInfo_t serverThreadInfo;
	pthread_t serverThread = 0;

	if(pipe(sockCommPipes) == -1){
		logMsg(__func__, __LINE__, ERROR, "Error in pipes opening. Abort.");
		return EXIT_FAILURE;
	}
	// create TCP listener socket
	if(createServTCPSocket(&tcpSockaddr, &socketFd, listenPort)){
		logMsg(__func__, __LINE__, ERROR, "Error in creating server TCP socket. Abort.");

		for(int i = 0; i < 2; i++)
			close(sockCommPipes[i]);
		return EXIT_FAILURE;
	}

	serverThreadInfo.inputCommsPipeFd = sockCommPipes[1];
	serverThreadInfo.socketFd = socketFd;
	serverThreadInfo.conf = serverConf_ptr;

	// start server TCP listener pthread
	if( pthread_create(&serverThread, NULL, startListenTCPSocket, (void*) &serverThreadInfo) ){
		// pthread creating error
		logMsg(__func__, __LINE__, ERROR, "Error in start \"startListenTCPSocket\" pthread. Abort.");

		// close commands pipes
		for(int i = 0; i < 2; i++)
			close(sockCommPipes[i]);
		return EXIT_FAILURE;
	}
	else{
		// if pthread creating was successful
		close(sockCommPipes[1]);

		logMsg(__func__, __LINE__, INFO, "\"startListenTCPSocket\" pthread started successful. Join pthread.");
		pthread_join(serverThread, NULL);
	}

	return EXIT_SUCCESS;
}

/**
 * Server TCP listener pthread function.
 * Work with while statement. Run accept() for input clients. Then create
 * new pthreads for all input clients.
 */
void* startListenTCPSocket(void* threadData){
	logMsg(__func__, __LINE__, INFO, "Start server TCP listener pthread.");

	serverCommands_t currCommAction = LISTEN;
	serverSysInfo_t* serverInfo = (serverSysInfo_t*) threadData;

	pthread_t connectedPeersArr[10];		//TODO: hardcoded number of peers!
	int connectedPeersNum = 0;

	listen(serverInfo->socketFd, 1);		//TODO: hardcoded nuber of backlog!

	while(currCommAction != STOP_SERVER){

		if(currCommAction == LISTEN){
			struct sockaddr_in tcpInputClient_addr;
			int inputClientFd = 0;
			socklen_t addrLen = sizeof(tcpInputClient_addr);
			if ((inputClientFd = accept(serverInfo->socketFd, (struct sockaddr *)&tcpInputClient_addr, &addrLen)) < 0){
				logMsg(__func__, __LINE__, ERROR, "Error in accepting input peer. RC: %d", inputClientFd);
				close(serverInfo->socketFd);
				pthread_exit((void*) EXIT_FAILURE);
			}

			logMsg(__func__, __LINE__, INFO, "Accepted connection from: %s", inet_ntoa(tcpInputClient_addr.sin_addr));

			// TODO: Грязный лайфхак! Надо переделать
			serverSysInfo_t peerInfo;
			peerInfo.conf = serverInfo->conf;
			peerInfo.socketFd = inputClientFd;
			if( pthread_create(&(connectedPeersArr[connectedPeersNum]), NULL, startPeerThread, (void*) &peerInfo) ){
				// pthread creating error
				logMsg(__func__, __LINE__, ERROR, "Error in creating peer pthread.");

				// close accepted socket
				close(serverInfo->socketFd);
				pthread_exit((void*) EXIT_FAILURE);
			}
			else{
				// if pthread creating was successful
			}

		}
	}


	pthread_exit((void*) EXIT_SUCCESS);
}

/**
 * Input client (peer) pthread function.
 * Protocol:
 * After connection client send:
 * 1) client send password: char[MAX_PASS_LEN]
 * 2) client send file info struct: file_info_msg_t
 * 3) client send file md5 hash
 * 4) client send file
 */
void* startPeerThread(void* threadData){
	logMsg(__func__, __LINE__, INFO, "Start peer pthread.");
	serverSysInfo_t* connectInfo = (serverSysInfo_t*) threadData;
	char* serv_passw = connectInfo->conf->password;		// server password
	char* servDownloadFolder = connectInfo->conf->storageFolderPath;
	int inputConnectFd = connectInfo->socketFd;				// input connection FD
	ssize_t inputMsgSize = 0;

	// get password from client
	char* input_passw_buff = (char*) malloc((MAX_PASS_LEN + 1) * sizeof(char));
	memset(input_passw_buff, 0, (MAX_PASS_LEN + 1) * sizeof(char));
	if(recvData(inputConnectFd, (MAX_PASS_LEN + 1) * sizeof(char), &input_passw_buff) == -1){
		logMsg(__func__, __LINE__, ERROR, "Peer password receiving error. Abort peer connection. %s", input_passw_buff);
		free(input_passw_buff);
		pthread_exit(NULL);
	}

	// check input client password
	bool_t is_passw_correct = checkPassword(serv_passw, input_passw_buff);
	logMsg(__func__, __LINE__, INFO, "Received peer password: %s - %s",
			input_passw_buff, is_passw_correct == FALSE ? "wrong" : "correct");

	if(is_passw_correct == FALSE){
		// if wrong password - close connection and close pthread
		close(inputConnectFd);
		free(input_passw_buff);
		pthread_exit(NULL);
	}
	free(input_passw_buff);

	inputMsgSize = 0;

	// get file info struct
	file_info_msg_t recvInputFileInfo;
	size_t inputMsgBuffSize = sizeof(char) * MAX_FILESIZE_CHAR_NUM + sizeof(char) * MAX_FILENAME_LEN + 1;
	char* fileInfoMsgBuff = (char*) malloc(inputMsgBuffSize);
	memset(fileInfoMsgBuff, 0, inputMsgBuffSize);

	if((inputMsgSize = recvfrom(inputConnectFd, fileInfoMsgBuff, inputMsgBuffSize, 0, NULL, 0)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Peer file info message receiving error. Abort peer connection.");
		// Receiving error
		// TODO: free mem and close descriptors
		free(fileInfoMsgBuff);
		pthread_exit(NULL);
	}

	// deserialize input file info message
	if(deserialize_FileInfoMsg(&recvInputFileInfo, fileInfoMsgBuff, ';')){
		logMsg(__func__, __LINE__, ERROR, "Deserialize input file info message error. Abort peer connection.");
		// Deserialize error
		// TODO: free mem and close descriptors
		free(fileInfoMsgBuff);
		pthread_exit(NULL);
	}

	logMsg(__func__, __LINE__, INFO, "Received file info msg of peer. Filesize %lld; Filename: %s",
			recvInputFileInfo.fileSize, recvInputFileInfo.fileName);

	// builds full file path
	size_t recvFuleFullPathSize = strlen(servDownloadFolder) * sizeof(char) + strlen(recvInputFileInfo.fileName) * sizeof(char) + 1;
	char* recvFileLocalFullPath = (char*) malloc(recvFuleFullPathSize);
	memset(recvFileLocalFullPath, 0, recvFuleFullPathSize);
	strncpy(recvFileLocalFullPath, servDownloadFolder, strlen(servDownloadFolder) * sizeof(char));
	strncpy(&recvFileLocalFullPath[strlen(servDownloadFolder)], recvInputFileInfo.fileName, strlen(recvInputFileInfo.fileName) * sizeof(char));

	// open new file to write mode
	logMsg(__func__, __LINE__, INFO, "Try to open tmp file in storage: %s", recvFileLocalFullPath);
#ifdef __linux__
	int localFileDescr = open(recvFileLocalFullPath, O_CREAT | O_WRONLY, S_IRUSR | S_IRGRP | S_IROTH);
#endif
	if(localFileDescr == -1){
		logMsg(__func__, __LINE__, ERROR, "Can't create local tmp file. Abort peer connection.");
		// Local file opening error
		// TODO: free mem and close descriptors
		pthread_exit(NULL);
	}

	// init MD5 hash checker
	MD5_CTX ctx;
	BYTE downloadedFileHash_md5[MD5_BLOCK_SIZE];
	memset(downloadedFileHash_md5, '\0', MD5_BLOCK_SIZE);
	md5_init(&ctx);

	inputMsgSize = 0;

	// get file md5 hash from peer
	BYTE peerFileMd5Hash[MD5_BLOCK_SIZE];
	memset(peerFileMd5Hash, '\0', MD5_BLOCK_SIZE * sizeof(BYTE));
	char* peerFileMd5HashStr = (char*) malloc((MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	memset(peerFileMd5HashStr, '\0', (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	if((inputMsgSize = recvfrom(inputConnectFd, peerFileMd5HashStr, (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char), 0, NULL, 0)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Peer file md5 hash message receiving error. Abort peer connection.");
		// Receiving error
		// TODO: free mem and close descriptors
		pthread_exit(NULL);
	}

	logMsg(__func__, __LINE__, INFO, "Received hash:\t %s", peerFileMd5HashStr);
	logMsg(__func__, __LINE__, INFO, "Start file transferring");

	// recv file
	file_size_t recvTotalBytes = 0;
	file_size_t fullRemainFileSize = recvInputFileInfo.fileSize;
	char* inputMsgBuff = (char*) malloc(SENDING_FILE_PACKET_SIZE * sizeof(char));
	memset(inputMsgBuff, '\0', SENDING_FILE_PACKET_SIZE * sizeof(char));
	inputMsgSize = 0;
	memset(inputMsgBuff, '\0', SENDING_FILE_PACKET_SIZE * sizeof(char));
	while( (inputMsgSize = recvfrom(inputConnectFd, inputMsgBuff, SENDING_FILE_PACKET_SIZE * sizeof(char), 0, NULL, 0)) > 0){
		// update MD5 hash checker
		recvTotalBytes += inputMsgSize;
		md5_update(&ctx, inputMsgBuff, inputMsgSize);

		// write input data into local file
		if(write(localFileDescr, inputMsgBuff, inputMsgSize) < 0){
			logMsg(__func__, __LINE__, ERROR, "Error in write receiving file data into local file. Abort peer connection.");
			// File writing error
			// TODO: free mem and close descriptors
			pthread_exit(NULL);
		}

		if(recvTotalBytes >= fullRemainFileSize)
			break;

		inputMsgSize = 0;
		memset(inputMsgBuff, '\0', SENDING_FILE_PACKET_SIZE * sizeof(char));
	}

	// count final hash for downloaded file
	md5_final(&ctx, downloadedFileHash_md5);

	// convert input md5 hash to byte format
	char* downloadedHash = (char*) malloc((MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	memset(downloadedHash, '\0', (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	fromByteArrToHexStr(downloadedFileHash_md5, MD5_BLOCK_SIZE, &downloadedHash);
	logMsg(__func__, __LINE__, INFO, "Counted hash:\t %s", downloadedHash);
	fromHexStrToByteArr(peerFileMd5HashStr, MD5_BLOCK_SIZE * 2, peerFileMd5Hash);

	netmsg_sending_res_t fileTransferRes;
	// compare received hash and counted hash of downloaded file
	if(cmpHash_md5(peerFileMd5Hash, downloadedFileHash_md5)){
		logMsg(__func__, __LINE__, INFO, "The md5 files hashes are different. Abort peer connection.");
		// hashes are different - error
		fileTransferRes = FAIL;
		// close and delete local file
		// output log msg about error
	}
	else{
		logMsg(__func__, __LINE__, INFO, "The md5 files hashes are same.");
		// hashes are same
		fileTransferRes = SUCCESSFUL;
		// sync downloaded file
		fsync(localFileDescr);
	}

	logMsg(__func__, __LINE__, INFO, "Transfer is %s. Close connection.",
			fileTransferRes == FAIL ? "unsuccessful" : "successful");

	/*
	if(send(inputConnectFd, &fileTransferRes, sizeof(fileTransferRes), 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending message to peer about file getting result. Abort peer connection.");
		// Sending result message error
		// TODO: free mem and close descriptors
		return EXIT_FAILURE;
	}
	*/

	// close pthread
	close(inputConnectFd);
	close(localFileDescr);
	free(peerFileMd5HashStr);
	free(downloadedHash);
	free(fileInfoMsgBuff);
	free(recvFileLocalFullPath);
	free(inputMsgBuff);

	pthread_exit(NULL);
}

ssize_t recvData(int _socked_fd, size_t _recv_data_size, OUT_ARG char** _recv_buff){
	ssize_t input_data_size = 0;

	if((input_data_size = recvfrom(_socked_fd, *_recv_buff, _recv_data_size, 0, NULL, 0)) < 0){
		input_data_size = -1;
	}

	return input_data_size;
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

	if((*_socket_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return EXIT_FAILURE;

    memset(_tcpsocket_addr, 0, sizeof(*_tcpsocket_addr));
    (*_tcpsocket_addr).sin_family      = AF_INET;
    (*_tcpsocket_addr).sin_addr.s_addr = htonl(INADDR_ANY);
    (*_tcpsocket_addr).sin_port        = htons(_port);

    if( bind(*_socket_desc, (struct sockaddr *)_tcpsocket_addr, sizeof(*_tcpsocket_addr)) < 0 )
    	return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

