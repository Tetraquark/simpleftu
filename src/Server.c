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
	socket_t socketFd = 0;
	int listenPort = serverConf_ptr->port;
	int sockCommPipes[2];
	serverSysInfo_t serverThreadInfo;
	thread_t serverThread = 0;
	int thread_start_rc = 0;

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
#ifdef _WIN32
	serverThread = CreateThread(NULL, 0, startListenTCPSocket, &serverThreadInfo, 0, NULL);
	thread_start_rc = serverThread;
#elif __linux__
	thread_start_rc = pthread_create(&serverThread, NULL, startListenTCPSocket, (void*) &serverThreadInfo);
#endif

	if( thread_start_rc ){
		// thread creating error
		logMsg(__func__, __LINE__, ERROR, "Error in start \"startListenTCPSocket\" pthread. Abort.");

		//TODO: close socket!

		// close commands pipes
		for(int i = 0; i < 2; i++)
			close(sockCommPipes[i]);
		return EXIT_FAILURE;
	}
	else{
		// if pthread creating was successful
		close(sockCommPipes[1]);

		logMsg(__func__, __LINE__, INFO, "\"startListenTCPSocket\" pthread started successful. Join pthread.");
		joinThread_inf(serverThread);
	}

	//TODO: close socket!

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
			socket_t inputClientFd = 0;
			socklen_t addrLen = sizeof(tcpInputClient_addr);
			if ((inputClientFd = accept(serverInfo->socketFd, (struct sockaddr *)&tcpInputClient_addr, &addrLen)) < 0){
				logMsg(__func__, __LINE__, ERROR, "Error in accepting input peer. RC: %d", inputClientFd);
				closeSocket(serverInfo->socketFd);
				exitThread(EXIT_FAILURE);
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
				exitThread(EXIT_FAILURE);
			}
			else{
				// if pthread creating was successful
			}

		}
	}

	exitThread(EXIT_SUCCESS);
}

static file_size_t __recvAndSaveFile(int _socket_fd, char* _tmp_file_fullpath, file_size_t full_remain_fileSize,
		OUT_ARG BYTE _counted_tmpfile_hash_md5[MD5_BLOCK_SIZE]){
	file_size_t recv_total_data_size = 0;
	int tmp_file_fd = 0;
	MD5_CTX ctx;
	ssize_t recv_packet_size = 0;
	char* input_dataBuff = NULL;

	input_dataBuff = (char*) malloc(SENDING_FILE_PACKET_SIZE * sizeof(char));
	memset(input_dataBuff, '\0', SENDING_FILE_PACKET_SIZE * sizeof(char));

	// open new file to write mode
	logMsg(__func__, __LINE__, INFO, "Try to open tmp file in storage: %s", _tmp_file_fullpath);

	tmp_file_fd = open(_tmp_file_fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IRGRP | S_IROTH);
	if(tmp_file_fd == -1){
		logMsg(__func__, __LINE__, ERROR,
				"Can't create local tmp file for saving. Abort peer connection.");
		free(input_dataBuff);
		return -1;
	}

	// init MD5 hash checker
	md5_init(&ctx);

	// recv file
	while( (recv_packet_size = recvfrom(_socket_fd, input_dataBuff, SENDING_FILE_PACKET_SIZE * sizeof(char), 0, NULL, 0)) > 0){
		recv_total_data_size += recv_packet_size;

		// update MD5 hash checker
		md5_update(&ctx, input_dataBuff, recv_packet_size);

		// write input data into local file
		if(write(tmp_file_fd, input_dataBuff, recv_packet_size) < 0){
			logMsg(__func__, __LINE__, ERROR,
					"Error in write receiving file data into local file. Abort peer connection.");
			// File writing error
			recv_total_data_size = -1;
			break;
		}

		if(recv_total_data_size >= full_remain_fileSize){
			//recv_total_data_size = -1; ??
			break;
		}

		recv_packet_size = 0;
		memset(input_dataBuff, '\0', SENDING_FILE_PACKET_SIZE * sizeof(char));
	}

	// count final hash for downloaded file
	md5_final(&ctx, _counted_tmpfile_hash_md5);

	fsync(tmp_file_fd);

	free(input_dataBuff);
	close(tmp_file_fd);
	return recv_total_data_size;
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
	char* serv_storage_dir_path = connectInfo->conf->storageFolderPath;
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
	file_info_msg_t recv_fileInfo_strc;
	size_t input_msg_buffSize = sizeof(char) * MAX_FILESIZE_CHAR_NUM +
			sizeof(char) * MAX_FILENAME_LEN + 1;
	char* fileInfo_msgBuff = (char*) malloc(input_msg_buffSize);
	memset(fileInfo_msgBuff, '\0', input_msg_buffSize);

	if(recvData(inputConnectFd, input_msg_buffSize, &fileInfo_msgBuff) == -1){
		logMsg(__func__, __LINE__, ERROR,
				"Peer file info message receiving error. Abort peer connection.");
		close(inputConnectFd);
		free(fileInfo_msgBuff);
		pthread_exit(NULL);
	}

	// deserialize input file info message
	if(deserialize_FileInfoMsg(&recv_fileInfo_strc, fileInfo_msgBuff, ';')){
		logMsg(__func__, __LINE__, ERROR,
				"Deserialize input file info message error. Abort peer connection.");
		close(inputConnectFd);
		free(fileInfo_msgBuff);
		pthread_exit(NULL);
	}
	free(fileInfo_msgBuff);

	logMsg(__func__, __LINE__, INFO, "Received file info msg of peer. Filesize %lld; Filename: %s",
			recv_fileInfo_strc.fileSize, recv_fileInfo_strc.fileName);

	// builds full tmp file path
	size_t tmp_file_fullpath_size = strlen(serv_storage_dir_path) * sizeof(char)
			+ strlen(recv_fileInfo_strc.fileName) * sizeof(char) + 1;
	char* tmp_file_fullpath_str = (char*) malloc(tmp_file_fullpath_size);
	memset(tmp_file_fullpath_str, '\0', tmp_file_fullpath_size);

	// copy storage dir path to tmp file path
	strncpy(tmp_file_fullpath_str, serv_storage_dir_path,
			strlen(serv_storage_dir_path) * sizeof(char));
	// copy input file name to tmp file path
	strncpy(&tmp_file_fullpath_str[strlen(serv_storage_dir_path)],
			recv_fileInfo_strc.fileName, strlen(recv_fileInfo_strc.fileName) * sizeof(char));

	inputMsgSize = 0;

	// get file md5 hash from peer
	BYTE peer_fileHash_md5[MD5_BLOCK_SIZE];
	memset(peer_fileHash_md5, '\0', MD5_BLOCK_SIZE * sizeof(BYTE));
	char* peer_fileHash_md5_str = (char*) malloc((MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	memset(peer_fileHash_md5_str, '\0', (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	if(recvData(inputConnectFd, (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char), &peer_fileHash_md5_str) == -1){
		logMsg(__func__, __LINE__, ERROR,
				"Peer file md5 hash message receiving error. Abort peer connection.");
		close(inputConnectFd);
		free(peer_fileHash_md5_str);
		pthread_exit(NULL);
	}
	fromHexStrToByteArr(peer_fileHash_md5_str, MD5_BLOCK_SIZE * 2, peer_fileHash_md5);
	logMsg(__func__, __LINE__, INFO, "Received hash:\t %s", peer_fileHash_md5_str);
	free(peer_fileHash_md5_str);

	logMsg(__func__, __LINE__, INFO, "Start file transferring");
	BYTE tmp_fileHash_md5[MD5_BLOCK_SIZE];
	memset(tmp_fileHash_md5, '\0', MD5_BLOCK_SIZE);

	// recv file
	file_size_t recvTotalBytes = 0;
	recvTotalBytes = __recvAndSaveFile(inputConnectFd, tmp_file_fullpath_str, recv_fileInfo_strc.fileSize, tmp_fileHash_md5);
	if(recvTotalBytes == -1){
		logMsg(__func__, __LINE__, ERROR, "Error file transferring. Abort.");
		close(inputConnectFd);
		pthread_exit(NULL);
	}

	// convert counted tmp file md5 hash to byte format
	char* tmp_fileHash_md5_str = (char*) malloc((MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	memset(tmp_fileHash_md5_str, '\0', (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	fromByteArrToHexStr(tmp_fileHash_md5, MD5_BLOCK_SIZE, &tmp_fileHash_md5_str);
	logMsg(__func__, __LINE__, INFO, "Counted hash:\t %s", tmp_fileHash_md5_str);

	netmsg_sending_res_t fileTransferRes;
	// compare received hash and counted hash of downloaded file
	if(cmpHash_md5(peer_fileHash_md5, tmp_fileHash_md5)){
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
	free(tmp_fileHash_md5_str);

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
int createServTCPSocket(struct sockaddr_in* _tcpsocket_addr, socket_t* _socket_desc, int _port){

	*_socket_desc = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
	if(*_socket_desc == INVALID_SOCKET)
#elif __linux
	if(*_socket_desc < 0)
#else
#endif
		return EXIT_FAILURE;

    memset(_tcpsocket_addr, 0, sizeof(*_tcpsocket_addr));
    (*_tcpsocket_addr).sin_family      = AF_INET;
    (*_tcpsocket_addr).sin_addr.s_addr = htonl(INADDR_ANY);
    (*_tcpsocket_addr).sin_port        = htons(_port);

    if( bind(*_socket_desc, (struct sockaddr *)_tcpsocket_addr, sizeof(*_tcpsocket_addr)) != 0 )
    	return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

