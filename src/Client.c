/*
 * Client.c
 *
 *  Created on: 3 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Client.h"

#ifdef DEBUG
/**
 * Protocol:
 * After connection client send:
 * 1) client send password: char[MAX_PASS_LEN]
 * 2) client send file info struct: file_info_msg_t
 * 3) client send file md5 hash
 * 4) client send file
 */
int DEBUG_sendTestFile(char* serv_ip, int serv_port, char sendingfile_path[MAX_FULL_FILE_PATH_LEN]){
	logMsg(__func__, __LINE__, INFO, "Run DEBUG_sendTestFile function for test file transfer.");

	struct sockaddr_in tcpsocket_addr_strct;
	int socketDescr = 0;

	MD5_CTX ctx;

	// open file
	struct stat fileStatbuff;
	file_size_t fileSize = 0;
	int fd = open(sendingfile_path, O_RDONLY);
	if(fd == -1){
		logMsg(__func__, __LINE__, ERROR, "Cant open %s file.", sendingfile_path);
		return EXIT_FAILURE;
	}

	// get file stats info
	if ((fstat(fd, &fileStatbuff) != 0) || (!S_ISREG(fileStatbuff.st_mode))) {
		logMsg(__func__, __LINE__, ERROR, "Cant get file stats.");
		close(fd);
		return EXIT_FAILURE;
	}
	fileSize = fileStatbuff.st_size;

	// init md5 hasher
	md5_init(&ctx);

	initTcpConnSocket(&tcpsocket_addr_strct, &socketDescr, serv_ip, serv_port);

	if(connect(socketDescr, (struct sockaddr *) &tcpsocket_addr_strct, sizeof(tcpsocket_addr_strct)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in connect to server.");
		close(fd);
		return EXIT_FAILURE;
	}

	// send password
	char* passw = (char*) malloc(MAX_PASS_LEN + 1 * sizeof(char));
	memcpy(passw, DEBUG_PASSWORD, MAX_PASS_LEN + 1 * sizeof(char));
	logMsg(__func__, __LINE__, INFO, "Try to send password: %s", passw);
	if(send(socketDescr, passw, MAX_PASS_LEN + 1 * sizeof(char), 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending password to server. Abort connection.");
		// Sending result message error
		// TODO: free mem and close descriptors
		close(socketDescr);
		close(fd);
		return EXIT_FAILURE;
	}

	free(passw);

	// send file info
	file_info_msg_t fileInfoStruct;
#ifdef __linux__
	char* fileName_str = basename(sendingfile_path);
#endif
	memset(fileInfoStruct.fileName, '\0', 0);
	memcpy(fileInfoStruct.fileName, fileName_str, MAX_FILENAME_LEN * sizeof(char));
	fileInfoStruct.fileSize = fileSize;

	char* fileInfoMsg_ptr = NULL;
	ssize_t fileInfoMsgSize = serialize_FileInfoMsg(fileInfoStruct, ';', &fileInfoMsg_ptr);

	logMsg(__func__, __LINE__, INFO, "Try to send file info msg: %s", fileInfoMsg_ptr);

	if(send(socketDescr, fileInfoMsg_ptr, fileInfoMsgSize, 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending file info to server. Abort connection.");
		// Sending result message error
		// TODO: free mem and close descriptors
		close(socketDescr);
		close(fd);
		free(fileInfoMsg_ptr);
		return EXIT_FAILURE;
	}

	free(fileInfoMsg_ptr);

	file_size_t bytes_readed = 0;
	char* fdBuff = (char*) malloc(SENDING_FILE_PACKET_SIZE * sizeof(char));
	memset(fdBuff, '\0', SENDING_FILE_PACKET_SIZE);
	while( ((bytes_readed = read(fd, fdBuff, SENDING_FILE_PACKET_SIZE * sizeof(char))) > 0) ){
		md5_update(&ctx, fdBuff, bytes_readed);
		memset(fdBuff, '\0', SENDING_FILE_PACKET_SIZE);
	}

	BYTE hashArr[MD5_BLOCK_SIZE];
	memset(hashArr, '\0', MD5_BLOCK_SIZE * sizeof(BYTE));
	md5_final(&ctx, hashArr);

	char* hashArrStr = (char*) malloc(MD5_BLOCK_SIZE * 2 * sizeof(char));
	memset(hashArrStr, '\0', MD5_BLOCK_SIZE * 2 * sizeof(char));
	fromByteArrToHexStr(hashArr, MD5_BLOCK_SIZE, &hashArrStr);

	// send file md5 hash
	logMsg(__func__, __LINE__, INFO, "Send md5 hash: %s", hashArrStr);
	if(send(socketDescr, hashArrStr, MD5_BLOCK_SIZE * 2 * sizeof(char), 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending file md5 hash to server. Abort connection.");
		// Sending result message error
		// TODO: free mem and close descriptors
		close(socketDescr);
		close(fd);
		free(hashArrStr);
		return EXIT_FAILURE;
	}

	free(hashArrStr);

	// read file data block, count md5 hash, send file data block
	bytes_readed = 0;
	file_size_t total_bytes_sended = 0;
	file_size_t bytes_sended = 0;

	close(fd);
	fd = open(sendingfile_path, O_RDONLY);

	int _transfProgress = 0;
	while( ((bytes_readed = read(fd, fdBuff, SENDING_FILE_PACKET_SIZE * sizeof(char))) > 0) ){

		if( (bytes_sended = send(socketDescr, fdBuff, bytes_readed, 0)) < 0){
			logMsg(__func__, __LINE__, ERROR, "Error in sending file data block to server. Abort connection.");
			// Sending result message error
			// TODO: free mem and close descriptors
			close(socketDescr);
			close(fd);
			return EXIT_FAILURE;
		}

		total_bytes_sended += bytes_sended;

		_transfProgress = (total_bytes_sended * 100) / fileSize;
		// TODO: create transfer progress output

		bytes_sended = 0;
		memset(fdBuff, '\0', SENDING_FILE_PACKET_SIZE);
	}
	logMsg(__func__, __LINE__, INFO, "Total send file bytes: %lld", total_bytes_sended);

	logMsg(__func__, __LINE__, INFO, "File successfully transferred.");

	shutdown(socketDescr, SHUT_WR);
	//close(socketDescr);
	close(fd);
	return EXIT_SUCCESS;
}
#endif

int initTcpConnSocket(struct sockaddr_in* __tcpsocket_addr, int* __socket_desc, char* __server_addr, int __server_port){

	if((*__socket_desc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return EXIT_FAILURE;

    (*__tcpsocket_addr).sin_family      = AF_INET;
    (*__tcpsocket_addr).sin_addr.s_addr = inet_addr(__server_addr);
    (*__tcpsocket_addr).sin_port        = htons(__server_port);

    return EXIT_SUCCESS;
};
