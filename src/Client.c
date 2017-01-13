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
int DEBUG_sendTestFile(char* serv_ip, int serv_port, char sendingfile_path[MAX_FULL_FILE_PATH_LEN + 1], char* serv_pass){
	logMsg(__func__, __LINE__, INFO, "Start DEBUG_sendTestFile: serv_ip: %s ; serv_port: %d ; serv_pass: %s ; send_filepath: %s",
			serv_ip, serv_port, serv_pass, sendingfile_path);

	struct sockaddr_in tcpsocket_addr_strct;
	int socketDescr = 0;
	MD5_CTX ctx;

	// get file size
	file_size_t _fileSize = 0;
	_fileSize = getFileSize(sendingfile_path);
	if(_fileSize == -1){
		logMsg(__func__, __LINE__, ERROR, "Cant open %s file.", sendingfile_path);
		return EXIT_FAILURE;
	}

	// open file for sending
	int fd = open(sendingfile_path, O_RDONLY);
	if(fd == -1){
		logMsg(__func__, __LINE__, ERROR, "Cant open %s file.", sendingfile_path);
		return EXIT_FAILURE;
	}

	// init md5 hasher
	md5_init(&ctx);

	// init tcp socket
	if(initTcpConnSocket(&tcpsocket_addr_strct, &socketDescr, serv_ip, serv_port)){
		logMsg(__func__, __LINE__, ERROR, "Error creating TCP socket. Abort.");
		close(fd);
		return EXIT_FAILURE;
	}

	if(connect(socketDescr, (struct sockaddr *) &tcpsocket_addr_strct, sizeof(tcpsocket_addr_strct)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in connect to server.");
		close(fd);
		return EXIT_FAILURE;
	}

	// send password
	logMsg(__func__, __LINE__, INFO, "Try to send password: %s", serv_pass);
	if(send(socketDescr, serv_pass, MAX_PASS_LEN * sizeof(char), 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending password to server. Abort connection.");
		// Sending result message error
		// TODO: free mem and close descriptors
		close(socketDescr);
		close(fd);
		return EXIT_FAILURE;
	}

	// send file info
	file_info_msg_t fileInfoStruct;
	// get file name from path
	char* _fileName_str = getFileNameFromPath(sendingfile_path);

	if(_fileName_str != NULL){
		memset(fileInfoStruct.fileName, '\0', MAX_FILENAME_LEN * sizeof(char));
		// TODO: add sizes cmp for cstrs
		memcpy(fileInfoStruct.fileName, _fileName_str, strlen(_fileName_str) * sizeof(char));
		fileInfoStruct.fileSize = _fileSize;
		free(_fileName_str);
	}
	else{
		logMsg(__func__, __LINE__, ERROR, "Error. Can't get name of file from input path string. Abort.");
		// TODO: free mem and close descriptors
		close(socketDescr);
		close(fd);
		return EXIT_FAILURE;
	}

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
	char* fdBuff = (char*) malloc((SENDING_FILE_PACKET_SIZE) * sizeof(char));
	memset(fdBuff, '\0', (SENDING_FILE_PACKET_SIZE) * sizeof(char));
	while( ((bytes_readed = read(fd, fdBuff, SENDING_FILE_PACKET_SIZE * sizeof(char))) > 0) ){
		md5_update(&ctx, fdBuff, bytes_readed);
		memset(fdBuff, '\0', (SENDING_FILE_PACKET_SIZE) * sizeof(char));
	}

	BYTE hashArr[MD5_BLOCK_SIZE];
	memset(hashArr, '\0', MD5_BLOCK_SIZE * sizeof(BYTE));
	md5_final(&ctx, hashArr);

	char* hashArrStr = (char*) malloc((MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	memset(hashArrStr, '\0', (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	fromByteArrToHexStr(hashArr, MD5_BLOCK_SIZE, &hashArrStr);

	// send file md5 hash
	logMsg(__func__, __LINE__, INFO, "Send md5 hash: %s", hashArrStr);
	if(send(socketDescr, hashArrStr, (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char), 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending file md5 hash to server. Abort connection.");
		// Sending result message error
		// TODO: free mem and close descriptors
		close(socketDescr);
		close(fd);
		free(hashArrStr);
		return EXIT_FAILURE;
	}

	// read file data block, count md5 hash, send file data block
	bytes_readed = 0;
	file_size_t total_bytes_sended = 0;
	file_size_t bytes_sended = 0;

	logMsg(__func__, __LINE__, INFO, "Start file transferring");

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

		_transfProgress = (total_bytes_sended * 100) / _fileSize;
		// TODO: create transfer progress output

		bytes_sended = 0;
		memset(fdBuff, '\0', SENDING_FILE_PACKET_SIZE);
	}
	logMsg(__func__, __LINE__, INFO, "Total send file bytes: %lld", total_bytes_sended);

	logMsg(__func__, __LINE__, INFO, "File successfully transferred.");

	shutdown(socketDescr, SHUT_WR);
	//close(socketDescr);
	free(hashArrStr);
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
