/*
 * server.c
 *
 *  Created on: 8 окт. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Server.h"

int startServTCPListener(serverConfig_t* _serv_conf_ptr){
	logMsg(__func__, __LINE__, LOG_INFO, "Start server TCP Listener.");

	struct sockaddr_in serv_sockaddr;
	socket_t serv_socket_d = 0;
	int listenPort = _serv_conf_ptr->port;
#ifdef __linux__
	int serv_commands_pipes[2];
#endif
	serverSysInfo_t serv_threadInfo;
	thread_t serv_thread = 0;
#ifdef _WIN32
	HANDLE thread_start_rc;
#elif __linux__
	int thread_start_rc = 0;
#endif

#ifdef __linux__
	if(pipe(serv_commands_pipes) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in pipes opening. Abort.");
		return EXIT_FAILURE;
	}
#endif
	// create TCP listener socket
	if(socket_createServTCP(listenPort, &serv_sockaddr, &serv_socket_d)){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in creating server TCP socket. Abort.");
#ifdef __linux__
		for(int i = 0; i < 2; i++)
			close(serv_commands_pipes[i]);
#endif
		return EXIT_FAILURE;
	}

#ifdef __linux__
	serv_threadInfo.inputCommsPipeFd = serv_commands_pipes[1];
#endif
	serv_threadInfo.socket_d = serv_socket_d;
	serv_threadInfo.conf = _serv_conf_ptr;

	// start server TCP listener pthread
#ifdef _WIN32
	serv_thread = CreateThread(NULL, 0, startListenTCPSocket, &serv_threadInfo, 0, NULL);
	thread_start_rc = serv_thread;
#elif __linux__
	thread_start_rc = pthread_create(&serv_thread, NULL, startListenTCPSocket, (void*) &serv_threadInfo);
#endif

#ifdef _WIN32
	if( NULL == thread_start_rc ){
#elif __linux__
	if( 0 != thread_start_rc ){
#endif
		// thread creating error
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in start \"startListenTCPSocket\" pthread. Abort.");

		socket_close(serv_socket_d);

#ifdef __linux__
		// close commands pipes
		for(int i = 0; i < 2; i++)
			close(serv_commands_pipes[i]);
#endif
		return EXIT_FAILURE;
	}
	else{
#ifdef __linux__
		// if pthread creating was successful
		close(serv_commands_pipes[1]);
#endif
		logMsg(__func__, __LINE__, LOG_INFO, "\"startListenTCPSocket\" pthread started successful. Join pthread.");
		joinThread_inf(serv_thread);
	}

	socket_close(serv_socket_d);
	return EXIT_SUCCESS;
}

/**
 * Server TCP listener pthread function.
 * Work with while statement. Run accept() for input clients. Then create
 * new pthreads for all input clients.
 */
thread_rc_t startListenTCPSocket(void* _thread_data_strc){
	logMsg(__func__, __LINE__, LOG_INFO, "Start server TCP listener pthread.");

	serverCommands_t curr_command = LISTEN;
	serverSysInfo_t* server_info = (serverSysInfo_t*) _thread_data_strc;

	thread_t connected_peersArr[10];		//TODO: hardcoded number of peers!
	int connected_peersNum = 0;
#ifdef _WIN32
	HANDLE thread_start_rc = 0;
#elif __linux__
	int thread_start_rc = 0;
#endif

	listen(server_info->socket_d, 1);		//TODO: hardcoded nuber of backlog!

	while(curr_command != STOP_SERVER){

		if(curr_command == LISTEN){
			struct sockaddr_in tcpInputClient_addr;
			socket_t inputClientFd = 0;
			socklen_t addrLen = sizeof(tcpInputClient_addr);
			if ((inputClientFd = accept(server_info->socket_d, (struct sockaddr *)&tcpInputClient_addr, &addrLen)) < 0){
				logMsg(__func__, __LINE__, LOG_ERROR, "Error in accepting input peer. RC: %d", inputClientFd);
				socket_close(server_info->socket_d);
				exitThread((void*) EXIT_FAILURE);
			}

			logMsg(__func__, __LINE__, LOG_INFO, "Accepted connection from: %s", inet_ntoa(tcpInputClient_addr.sin_addr));

			// TODO: Грязный лайфхак! Надо переделать
			serverSysInfo_t peerInfo;
			peerInfo.conf = server_info->conf;
			peerInfo.socket_d = inputClientFd;
#ifdef _WIN32
			connected_peersArr[connected_peersNum] = CreateThread(NULL, 0, startPeerThread, &peerInfo, 0, NULL);
			thread_start_rc = connected_peersArr[connected_peersNum];
#elif __linux__
			thread_start_rc = pthread_create(&(connected_peersArr[connected_peersNum]), NULL, startPeerThread, (void*) &peerInfo);
#endif

#ifdef _WIN32
			if( NULL == thread_start_rc ){
#elif __linux__
			if( 0 != thread_start_rc ){
#endif
				// pthread creating error
				logMsg(__func__, __LINE__, LOG_ERROR, "Error in creating peer pthread.");

				// close accepted socket
				socket_close(server_info->socket_d);
				exitThread((thread_rc_t) EXIT_FAILURE);
			}
			else{
				connected_peersNum++;
				// if pthread creating was successful
			}
		}
	}

	exitThread(EXIT_SUCCESS);
}

static file_size_t __recvAndSaveFile(int _socket_fd, char* _tmp_file_fullpath, file_size_t full_remain_fileSize,
		OUT_ARG BYTE _counted_tmpfile_hash_md5[MD5_BLOCK_SIZE]){

	file_size_t recv_total_data_size = 0;
#ifdef _WIN32
	FILE* tmp_file = NULL;
#elif __linux__
	int tmp_file = 0;
#endif

	MD5_CTX ctx;
	ssize_t recved_packet_size = 0;
	size_t input_packet_size = 0;
	char* input_dataBuff = NULL;

	input_dataBuff = (char*) malloc(SENDING_FILE_PACKET_SIZE * sizeof(char));
	memset(input_dataBuff, '\0', SENDING_FILE_PACKET_SIZE * sizeof(char));

	// open new file to write mode
	logMsg(__func__, __LINE__, LOG_INFO, "Try to open tmp file in storage: %s", _tmp_file_fullpath);

#ifdef _WIN32
	tmp_file = fopen(_tmp_file_fullpath, "wb");
	if(NULL == tmp_file){
		logMsg(__func__, __LINE__, LOG_ERROR,
				"Can't create local tmp file for saving. Abort peer connection.");
		free(input_dataBuff);
		return -1;
	}
#elif __linux__
	tmp_file = open(_tmp_file_fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IRGRP | S_IROTH);
	if(-1 == tmp_file){
		logMsg(__func__, __LINE__, LOG_ERROR,
				"Can't create local tmp file for saving. Abort peer connection.");
		free(input_dataBuff);
		return -1;
	}
#endif

	// init MD5 hash checker
	md5_init(&ctx);

	while(recv_total_data_size < full_remain_fileSize){

		if(socket_recvBytes(_socket_fd, sizeof(input_packet_size), (void*)&input_packet_size) == -1){
			logMsg(__func__, __LINE__, LOG_ERROR, "Error receiving input data packet size from socket. Abort peer connection.");
			recv_total_data_size = -1;
			break;
		}

		if((recved_packet_size = socket_recvBytes(_socket_fd, input_packet_size, (void*) input_dataBuff)) <= 0){
			logMsg(__func__, __LINE__, LOG_ERROR,
					"Error receiving input TCP message. Abort peer connection.");
			recv_total_data_size = -1;
			break;
		}

		// update MD5 hash checker
		md5_update(&ctx, input_dataBuff, recved_packet_size);

		// write input data into local file
#ifdef _WIN32
		if(fwrite(input_dataBuff, sizeof(char), recved_packet_size, tmp_file) != recved_packet_size){
#elif __linux__
		if(write(tmp_file, input_dataBuff, recved_packet_size) < 0){
#endif
			logMsg(__func__, __LINE__, LOG_ERROR,
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

	free(input_dataBuff);
#ifdef _WIN32
	fflush(tmp_file);
	fclose(tmp_file);
#elif __linux__
	fsync(tmp_file);
	close(tmp_file);
#endif
	return recv_total_data_size;
}

/**
 * File transfer protocol:
 * After connection client send:
 * 1) client sends password message size: msg_size_t
 * 2) client sends password: char[MAX_PASS_LEN]
 * 3) client receives result of password checking on server: netmsg_stat_code_t
 * 4) client sends the file info message size: msg_size_t
 * 5) client sends the file info struct (serialized cstring message): file_info_msg_t
 * 6) client receives result of deserialization file info message: netmsg_stat_code_t
 * 7) client sends the file md5 hash message size: msg_size_t
 * 8) client sends the file md5 hash: char[MD5_BLOCK_SIZE * 2]
 * 9) client receives result sending file md5 hash message: netmsg_stat_code_t
 * 10) client sends file: loop of sending data packet size and sending data packet
 * 11) client receives result of md5 hashes comparing and transfer result message: netmsg_stat_code_t
 */
thread_rc_t startPeerThread(void* _thread_data_strc){
	logMsg(__func__, __LINE__, LOG_INFO, "Start connected peer thread.");
	serverSysInfo_t* connect_info = (serverSysInfo_t*) _thread_data_strc;
	char* serv_passw = connect_info->conf->password;		// server password
	char* serv_storage_dir_path = connect_info->conf->storageFolderPath;
	int peer_socket_d = connect_info->socket_d;				// input connection socket descriptor
	msg_size_t input_msg_size = 0;
	netmsg_stat_code_t status_code = INCORRECT;

	// get input password message size
	if(socket_recvBytes(peer_socket_d, sizeof(input_msg_size), (void*)&input_msg_size) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error receiving input message size from socket. Abort peer connection.");
		socket_close(peer_socket_d);
		exitThread(NULL);
	}

	// get password message from client
	char* input_passw_buff = (char*) malloc(input_msg_size + 1);
	memset(input_passw_buff, 0, input_msg_size + 1);
	if(socket_recvBytes(peer_socket_d, input_msg_size, (void*) input_passw_buff) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Peer password receiving error. Abort peer connection. %s", input_passw_buff);
		free(input_passw_buff);

		socket_close(peer_socket_d);
		exitThread(NULL);
	}

	// check input client password
	bool_t is_passw_correct = checkPassword(serv_passw, input_passw_buff);
	logMsg(__func__, __LINE__, LOG_INFO, "Received peer password: %s - %s",
			input_passw_buff, is_passw_correct == B_FALSE ? "wrong" : "correct");

	if(is_passw_correct == B_FALSE){
		logMsg(__func__, __LINE__, LOG_INFO, "Abort peer connection.");

		status_code = INCORRECT;
		socket_sendBytes(peer_socket_d, &status_code, sizeof(status_code));

		// if wrong password - close connection and close pthread
		free(input_passw_buff);

		socket_close(peer_socket_d);
		exitThread(NULL);
	}
	free(input_passw_buff);

	// send answer message to peer
	status_code = CORRECT;
	if(socket_sendBytes(peer_socket_d, &status_code, sizeof(status_code)) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error sending answer message. Abort peer connection.");

		socket_close(peer_socket_d);
		exitThread(NULL);
	}

	input_msg_size = 0;
	// get input file info message size
	if(socket_recvBytes(peer_socket_d, sizeof(input_msg_size), (void*)&input_msg_size) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error receiving input message size from socket. Abort peer connection.");

		socket_close(peer_socket_d);
		exitThread(NULL);
	}

	// get file info message
	file_info_msg_t recv_fileInfo_strc;
	size_t input_msg_buffSize = sizeof(char) * input_msg_size;
	char* fileInfo_msgBuff = (char*) malloc(input_msg_buffSize);
	memset(fileInfo_msgBuff, '\0', input_msg_buffSize);

	if(socket_recvBytes(peer_socket_d, input_msg_buffSize, (void*) fileInfo_msgBuff) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR,
				"Peer file info message receiving error. Abort peer connection.");
		free(fileInfo_msgBuff);

		socket_close(peer_socket_d);
		exitThread(NULL);
	}
	//logMsg(__func__, __LINE__, LOG_INFO, "Get info of the input file: %s", fileInfo_msgBuff);

	// deserialize input file info message
	if(deserialize_FileInfoMsg(&recv_fileInfo_strc, fileInfo_msgBuff, ';')){
		logMsg(__func__, __LINE__, LOG_ERROR,
				"Deserialize input file info message error. Abort peer connection.");

		status_code = INCORRECT;
		socket_sendBytes(peer_socket_d, &status_code, sizeof(status_code));

		free(fileInfo_msgBuff);

		socket_close(peer_socket_d);
		exitThread(NULL);
	}
	free(fileInfo_msgBuff);

	// send answer message to peer
	status_code = CORRECT;
	if(socket_sendBytes(peer_socket_d, &status_code, sizeof(status_code)) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error sending answer message. Abort peer connection.");

		socket_close(peer_socket_d);
		exitThread(NULL);
	}

	logMsg(__func__, __LINE__, LOG_INFO, "Received the file info: file size %lld; file name: %s",
			recv_fileInfo_strc.fileSize, recv_fileInfo_strc.fileName);

	input_msg_size = 0;
	// get input file md5 hash message size
	if(socket_recvBytes(peer_socket_d, sizeof(input_msg_size), (void*)&input_msg_size) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error receiving input message size from socket. Abort peer connection.");

		socket_close(peer_socket_d);
		exitThread(NULL);
	}

	// TODO: maybe add cmp input_msg_size with MD5_BLOCK_SIZE * 2?

	// get file md5 hash from peer
	BYTE peer_fileHash_md5[MD5_BLOCK_SIZE];
	memset(peer_fileHash_md5, '\0', MD5_BLOCK_SIZE * sizeof(BYTE));
	char* peer_fileHash_md5_str = (char*) malloc(input_msg_size);
	memset(peer_fileHash_md5_str, '\0', input_msg_size);

	if(socket_recvBytes(peer_socket_d, input_msg_size, peer_fileHash_md5_str) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR,
				"Peer file md5 hash message receiving error. Abort peer connection.");
		free(peer_fileHash_md5_str);

		socket_close(peer_socket_d);
		exitThread(NULL);
	}
	fromHexStrToByteArr(peer_fileHash_md5_str, MD5_BLOCK_SIZE * 2, peer_fileHash_md5);
	logMsg(__func__, __LINE__, LOG_INFO, "Received hash:\t %s", peer_fileHash_md5_str);
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

	logMsg(__func__, __LINE__, LOG_INFO, "Start file transferring");
	BYTE tmp_fileHash_md5[MD5_BLOCK_SIZE];
	memset(tmp_fileHash_md5, '\0', MD5_BLOCK_SIZE);

	// send answer message to peer
	status_code = CORRECT;
	if(socket_sendBytes(peer_socket_d, &status_code, sizeof(status_code)) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error sending answer message. Abort peer connection.");

		socket_close(peer_socket_d);
		exitThread(NULL);
	}

	// recv file
	file_size_t recvTotalBytes = 0;
	recvTotalBytes = __recvAndSaveFile(peer_socket_d, tmp_file_fullpath_str, recv_fileInfo_strc.fileSize, tmp_fileHash_md5);
	if(recvTotalBytes == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error file transferring. Abort peer connection.");
		free(tmp_file_fullpath_str);

		socket_close(peer_socket_d);
		exitThread(NULL);
	}
	free(tmp_file_fullpath_str);

	// convert counted tmp file md5 hash to byte format
	char* tmp_fileHash_md5_str = (char*) malloc((MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	memset(tmp_fileHash_md5_str, '\0', (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	fromByteArrToHexStr(tmp_fileHash_md5, MD5_BLOCK_SIZE, &tmp_fileHash_md5_str);
	logMsg(__func__, __LINE__, LOG_INFO, "Counted hash:\t %s", tmp_fileHash_md5_str);

	// compare received hash and counted hash of downloaded file
	if(cmpHash_md5(peer_fileHash_md5, tmp_fileHash_md5)){
		logMsg(__func__, __LINE__, LOG_INFO, "The md5 files hashes are different. Abort peer connection.");
		// hashes are different - error

		// send answer message to peer
		status_code = INCORRECT;
	}
	else{
		logMsg(__func__, __LINE__, LOG_INFO, "The md5 files hashes are same.");
		// hashes are same
		status_code = CORRECT;
	}

	if(socket_sendBytes(peer_socket_d, &status_code, sizeof(status_code)) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error sending answer message. Abort peer connection.");

		free(tmp_fileHash_md5_str);

		socket_close(peer_socket_d);
		exitThread(NULL);
	}
	free(tmp_fileHash_md5_str);

	logMsg(__func__, __LINE__, LOG_INFO, "Transfer is %s. Close connection with peer.",
			status_code == INCORRECT ? "unsuccessful" : "successful");

	// close pthread
	socket_close(peer_socket_d);
	exitThread(NULL);
}

/**
 * Compare two char arrays with passwords.
 * @return B_TRUE if passwords is same
 */
bool_t checkPassword(const char* _serv_passw_str, const char* _in_passw_str){
	int cmp_res = 0;

	cmp_res = strncmp(_serv_passw_str, _in_passw_str, MAX_PASS_LEN * sizeof(char));

	if(!cmp_res)
		return B_TRUE;

	return B_FALSE;
}

