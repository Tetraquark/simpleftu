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
	if(socket_createServTCP(listenPort, &tcpSockaddr, &socketFd)){
		logMsg(__func__, __LINE__, ERROR, "Error in creating server TCP socket. Abort.");

		for(int i = 0; i < 2; i++)
			close(sockCommPipes[i]);
		return EXIT_FAILURE;
	}

	serverThreadInfo.inputCommsPipeFd = sockCommPipes[1];
	serverThreadInfo.socket_d = socketFd;
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
thread_rc_t startListenTCPSocket(void* _thread_data_strc){
	logMsg(__func__, __LINE__, INFO, "Start server TCP listener pthread.");

	serverCommands_t currCommAction = LISTEN;
	serverSysInfo_t* server_info = (serverSysInfo_t*) _thread_data_strc;

	pthread_t connectedPeersArr[10];		//TODO: hardcoded number of peers!
	int connectedPeersNum = 0;

	listen(server_info->socket_d, 1);		//TODO: hardcoded nuber of backlog!

	while(currCommAction != STOP_SERVER){

		if(currCommAction == LISTEN){
			struct sockaddr_in tcpInputClient_addr;
			socket_t inputClientFd = 0;
			socklen_t addrLen = sizeof(tcpInputClient_addr);
			if ((inputClientFd = accept(server_info->socket_d, (struct sockaddr *)&tcpInputClient_addr, &addrLen)) < 0){
				logMsg(__func__, __LINE__, ERROR, "Error in accepting input peer. RC: %d", inputClientFd);
				socket_close(server_info->socket_d);
				exitThread((void*) EXIT_FAILURE);
			}

			logMsg(__func__, __LINE__, INFO, "Accepted connection from: %s", inet_ntoa(tcpInputClient_addr.sin_addr));

			// TODO: Грязный лайфхак! Надо переделать
			serverSysInfo_t peerInfo;
			peerInfo.conf = server_info->conf;
			peerInfo.socket_d = inputClientFd;
			if( pthread_create(&(connectedPeersArr[connectedPeersNum]), NULL, startPeerThread, (void*) &peerInfo) ){
				// pthread creating error
				logMsg(__func__, __LINE__, ERROR, "Error in creating peer pthread.");

				// close accepted socket
				socket_close(server_info->socket_d);
				exitThread((thread_rc_t) EXIT_FAILURE);
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
	ssize_t recved_packet_size = 0;
	size_t input_packet_size = 0;
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

	while(recv_total_data_size < full_remain_fileSize){

		if(socket_recvBytes(_socket_fd, sizeof(input_packet_size), (void*)&input_packet_size) == -1){
			logMsg(__func__, __LINE__, ERROR, "Error receiving input data packet size from socket. Abort peer connection.");
			recv_total_data_size = -1;
			break;
		}

		if((recved_packet_size = socket_recvBytes(_socket_fd, input_packet_size, (void*) input_dataBuff)) <= 0){
			logMsg(__func__, __LINE__, ERROR,
					"Error receiving input TCP message. Abort peer connection.");
			recv_total_data_size = -1;
			break;
		}

		// update MD5 hash checker
		md5_update(&ctx, input_dataBuff, recved_packet_size);

		// write input data into local file
		if(write(tmp_file_fd, input_dataBuff, recved_packet_size) < 0){
			logMsg(__func__, __LINE__, ERROR,
					"Error in write receiving file data into local file. Abort peer connection.");
			// File writing error
			recv_total_data_size = -1;
			break;
		}

		recv_total_data_size += recved_packet_size;
		recved_packet_size = 0;
	}

	// count final hash for downloaded file
	md5_final(&ctx, _counted_tmpfile_hash_md5);

	fsync(tmp_file_fd);

	free(input_dataBuff);
	close(tmp_file_fd);
	return recv_total_data_size;
}

/**
 * File transfer protocol:
 * After connection client send:
 * 1) client sends password message size: size_t
 * 2) client sends password: char[MAX_PASS_LEN]
 * 3) client receives result of password checking on server: netmsg_stat_code_t
 * 4) client sends the file info message size: size_t
 * 5) client sends the file info struct (serialized cstring message): file_info_msg_t
 * 6) client receives result of deserialization file info message: netmsg_stat_code_t
 * 7) client sends the file md5 hash message size: size_t
 * 8) client sends the file md5 hash: char[MD5_BLOCK_SIZE * 2]
 * 9) client receives result sending file md5 hash message: netmsg_stat_code_t
 * 10) client sends file: loop of sending data packet size and sending data packet
 * 11) client receives result of md5 hashes comparing and transfer result message: netmsg_stat_code_t
 */
thread_rc_t startPeerThread(void* _thread_data_strc){
	logMsg(__func__, __LINE__, INFO, "Start peer pthread.");
	serverSysInfo_t* connect_info = (serverSysInfo_t*) _thread_data_strc;
	char* serv_passw = connect_info->conf->password;		// server password
	char* serv_storage_dir_path = connect_info->conf->storageFolderPath;
	int inputConnectFd = connect_info->socket_d;				// input connection FD
	ssize_t inputMsgSize = 0;
	netmsg_stat_code_t status_code = INCORRECT;

	// get input password message size
	if(socket_recvBytes(inputConnectFd, sizeof(inputMsgSize), (void*)&inputMsgSize) == -1){
		logMsg(__func__, __LINE__, ERROR, "Error receiving input message size from socket. Abort peer connection.");
		socket_close(inputConnectFd);
		exitThread(NULL);
	}

	// get password message from client
	char* input_passw_buff = (char*) malloc((inputMsgSize));
	memset(input_passw_buff, 0, inputMsgSize);
	if(socket_recvBytes(inputConnectFd, inputMsgSize, (void*) input_passw_buff) == -1){
		logMsg(__func__, __LINE__, ERROR, "Peer password receiving error. Abort peer connection. %s", input_passw_buff);
		free(input_passw_buff);

		socket_close(inputConnectFd);
		exitThread(NULL);
	}

	// check input client password
	bool_t is_passw_correct = checkPassword(serv_passw, input_passw_buff);
	logMsg(__func__, __LINE__, INFO, "Received peer password: %s - %s",
			input_passw_buff, is_passw_correct == FALSE ? "wrong" : "correct");

	if(is_passw_correct == FALSE){
		logMsg(__func__, __LINE__, INFO, "Abort peer connection.");

		status_code = INCORRECT;
		socket_sendBytes(inputConnectFd, &status_code, sizeof(status_code));

		// if wrong password - close connection and close pthread
		free(input_passw_buff);

		socket_close(inputConnectFd);
		exitThread(NULL);
	}
	free(input_passw_buff);

	// send answer message to peer
	status_code = CORRECT;
	if(socket_sendBytes(inputConnectFd, &status_code, sizeof(status_code)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error sending answer message. Abort peer connection.");

		socket_close(inputConnectFd);
		exitThread(NULL);
	}

	inputMsgSize = 0;
	// get input file info message size
	if(socket_recvBytes(inputConnectFd, sizeof(inputMsgSize), (void*)&inputMsgSize) == -1){
		logMsg(__func__, __LINE__, ERROR, "Error receiving input message size from socket. Abort peer connection.");

		socket_close(inputConnectFd);
		exitThread(NULL);
	}

	// get file info message
	file_info_msg_t recv_fileInfo_strc;
	size_t input_msg_buffSize = sizeof(char) * inputMsgSize;
	char* fileInfo_msgBuff = (char*) malloc(input_msg_buffSize);
	memset(fileInfo_msgBuff, '\0', input_msg_buffSize);

	if(socket_recvBytes(inputConnectFd, input_msg_buffSize, (void*) fileInfo_msgBuff) == -1){
		logMsg(__func__, __LINE__, ERROR,
				"Peer file info message receiving error. Abort peer connection.");
		free(fileInfo_msgBuff);

		socket_close(inputConnectFd);
		exitThread(NULL);
	}
	logMsg(__func__, __LINE__, INFO, "Get info of the input file: %s", fileInfo_msgBuff);

	// deserialize input file info message
	if(deserialize_FileInfoMsg(&recv_fileInfo_strc, fileInfo_msgBuff, ';')){
		logMsg(__func__, __LINE__, ERROR,
				"Deserialize input file info message error. Abort peer connection.");

		status_code = INCORRECT;
		socket_sendBytes(inputConnectFd, &status_code, sizeof(status_code));

		free(fileInfo_msgBuff);

		socket_close(inputConnectFd);
		exitThread(NULL);
	}
	free(fileInfo_msgBuff);

	// send answer message to peer
	status_code = CORRECT;
	if(socket_sendBytes(inputConnectFd, &status_code, sizeof(status_code)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error sending answer message. Abort peer connection.");

		socket_close(inputConnectFd);
		exitThread(NULL);
	}

	logMsg(__func__, __LINE__, INFO, "Received file info msg of peer. Filesize %lld; Filename: %s",
			recv_fileInfo_strc.fileSize, recv_fileInfo_strc.fileName);

	inputMsgSize = 0;
	// get input file md5 hash message size
	if(socket_recvBytes(inputConnectFd, sizeof(inputMsgSize), (void*)&inputMsgSize) == -1){
		logMsg(__func__, __LINE__, ERROR, "Error receiving input message size from socket. Abort peer connection.");

		socket_close(inputConnectFd);
		exitThread(NULL);
	}

	logMsg(__func__, __LINE__, INFO, "GET HASH: %ld", inputMsgSize);
	// TODO: maybe add cmp inputMsgSize with MD5_BLOCK_SIZE * 2?

	// get file md5 hash from peer
	BYTE peer_fileHash_md5[MD5_BLOCK_SIZE];
	memset(peer_fileHash_md5, '\0', MD5_BLOCK_SIZE * sizeof(BYTE));
	char* peer_fileHash_md5_str = (char*) malloc(inputMsgSize);
	memset(peer_fileHash_md5_str, '\0', inputMsgSize);

	if(socket_recvBytes(inputConnectFd, inputMsgSize, peer_fileHash_md5_str) == -1){
		logMsg(__func__, __LINE__, ERROR,
				"Peer file md5 hash message receiving error. Abort peer connection.");
		free(peer_fileHash_md5_str);

		socket_close(inputConnectFd);
		exitThread(NULL);
	}
	fromHexStrToByteArr(peer_fileHash_md5_str, MD5_BLOCK_SIZE * 2, peer_fileHash_md5);
	logMsg(__func__, __LINE__, INFO, "Received hash:\t %s", peer_fileHash_md5_str);
	free(peer_fileHash_md5_str);

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

	logMsg(__func__, __LINE__, INFO, "Start file transferring");
	BYTE tmp_fileHash_md5[MD5_BLOCK_SIZE];
	memset(tmp_fileHash_md5, '\0', MD5_BLOCK_SIZE);

	// send answer message to peer
	status_code = CORRECT;
	if(socket_sendBytes(inputConnectFd, &status_code, sizeof(status_code)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error sending answer message. Abort peer connection.");

		socket_close(inputConnectFd);
		exitThread(NULL);
	}

	// recv file
	file_size_t recvTotalBytes = 0;
	recvTotalBytes = __recvAndSaveFile(inputConnectFd, tmp_file_fullpath_str, recv_fileInfo_strc.fileSize, tmp_fileHash_md5);
	if(recvTotalBytes == -1){
		logMsg(__func__, __LINE__, ERROR, "Error file transferring. Abort peer connection.");
		free(tmp_file_fullpath_str);

		socket_close(inputConnectFd);
		exitThread(NULL);
	}
	free(tmp_file_fullpath_str);

	// convert counted tmp file md5 hash to byte format
	char* tmp_fileHash_md5_str = (char*) malloc((MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	memset(tmp_fileHash_md5_str, '\0', (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	fromByteArrToHexStr(tmp_fileHash_md5, MD5_BLOCK_SIZE, &tmp_fileHash_md5_str);
	logMsg(__func__, __LINE__, INFO, "Counted hash:\t %s", tmp_fileHash_md5_str);

	// compare received hash and counted hash of downloaded file
	if(cmpHash_md5(peer_fileHash_md5, tmp_fileHash_md5)){
		logMsg(__func__, __LINE__, INFO, "The md5 files hashes are different. Abort peer connection.");
		// hashes are different - error

		// send answer message to peer
		status_code = INCORRECT;
	}
	else{
		logMsg(__func__, __LINE__, INFO, "The md5 files hashes are same.");
		// hashes are same
		status_code = CORRECT;
	}

	if(socket_sendBytes(inputConnectFd, &status_code, sizeof(status_code)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error sending answer message. Abort peer connection.");

		socket_close(inputConnectFd);
		exitThread(NULL);
	}

	logMsg(__func__, __LINE__, INFO, "Transfer is %s. Close connection.",
			status_code == INCORRECT ? "unsuccessful" : "successful");

	/*
	if(send(inputConnectFd, &fileTransferRes, sizeof(fileTransferRes), 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending message to peer about file getting result. Abort peer connection.");
		// Sending result message error
		// TODO: free mem and close descriptors
		return EXIT_FAILURE;
	}
	*/

	// close pthread
	socket_close(inputConnectFd);
	free(tmp_fileHash_md5_str);

	exitThread(NULL);
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

