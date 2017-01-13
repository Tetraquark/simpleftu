/*
 * Client.c
 *
 *  Created on: 3 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Client.h"

/**
 * Protocol:
 * After connection client send:
 * 1) client send password: char[MAX_PASS_LEN]
 * 2) client send file info struct: file_info_msg_t
 * 3) client send file md5 hash
 * 4) client send file
 */
int startClient(char* _serv_ip, int _serv_port, char _sendingfile_path[MAX_FULL_FILE_PATH_LEN + 1], char* _serv_pass){
	logMsg(__func__, __LINE__, INFO, "Start DEBUG_sendTestFile: _serv_ip: %s ; _serv_port: %d ; _serv_pass: %s ; send_filepath: %s",
			_serv_ip, _serv_port, _serv_pass, _sendingfile_path);

	struct sockaddr_in tcpsocket_addr_strct;
	int socketDescr = 0;

	// init tcp socket
	if(initTcpConnSocket(&tcpsocket_addr_strct, &socketDescr, _serv_ip, _serv_port)){
		logMsg(__func__, __LINE__, ERROR, "Error creating TCP socket. Abort.");
		return EXIT_FAILURE;
	}

	// connect to server
	if(connect(socketDescr, (struct sockaddr *) &tcpsocket_addr_strct, sizeof(tcpsocket_addr_strct)) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in connect to server.");
		return EXIT_FAILURE;
	}

	// send password
	logMsg(__func__, __LINE__, INFO, "Try to send password: %s", _serv_pass);
	if(send(socketDescr, _serv_pass, MAX_PASS_LEN * sizeof(char), 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending password to server. Abort connection.");

		// TODO: free mem and close descriptors
		close(socketDescr);
		return EXIT_FAILURE;
	}

	// get the file size
	file_size_t file_size = 0;
	file_size = getFileSize(_sendingfile_path);
	if(file_size == -1){
		logMsg(__func__, __LINE__, ERROR, "Cant open %s file.", _sendingfile_path);
		return EXIT_FAILURE;
	}

	// get the file name from the path
	file_info_msg_t fileinfo_struct;
	char* _fileName_str = getFileNameFromPath(_sendingfile_path);
	if(_fileName_str != NULL){
		memset(fileinfo_struct.fileName, '\0', MAX_FILENAME_LEN * sizeof(char));
		// TODO: add sizes cmp for cstrs
		memcpy(fileinfo_struct.fileName, _fileName_str, strlen(_fileName_str) * sizeof(char));
		fileinfo_struct.fileSize = file_size;
		free(_fileName_str);
	}
	else{
		logMsg(__func__, __LINE__, ERROR, "Error. Can't get name of file from input path string. Abort.");

		// TODO: free mem and close descriptors
		close(socketDescr);
		return EXIT_FAILURE;
	}

	// send the file info
	char* fileinfo_msg_str = NULL;
	ssize_t fileinfo_msg_size = serialize_FileInfoMsg(fileinfo_struct, ';', &fileinfo_msg_str);
	logMsg(__func__, __LINE__, INFO, "Try to send file info msg: %s", fileinfo_msg_str);
	if(send(socketDescr, fileinfo_msg_str, fileinfo_msg_size, 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending file info to server. Abort connection.");

		// TODO: free mem and close descriptors
		close(socketDescr);
		free(fileinfo_msg_str);
		return EXIT_FAILURE;
	}
	free(fileinfo_msg_str);

	// count the file md5 hash
	BYTE hashArr[MD5_BLOCK_SIZE];
	memset(hashArr, '\0', MD5_BLOCK_SIZE * sizeof(BYTE));
	if(countFileHash_md5(_sendingfile_path, hashArr)){
		logMsg(__func__, __LINE__, ERROR, "Error in sending file info to server. Abort connection.");

		// TODO: free mem and close descriptors
		close(socketDescr);
		return EXIT_FAILURE;
	}

	// send the file md5 hash
	char* hashArr_str = (char*) malloc((MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	memset(hashArr_str, '\0', (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char));
	fromByteArrToHexStr(hashArr, MD5_BLOCK_SIZE, &hashArr_str);
	logMsg(__func__, __LINE__, INFO, "Send md5 hash: %s", hashArr_str);
	if(send(socketDescr, hashArr_str, (MD5_BLOCK_SIZE * 2 + 1) * sizeof(char), 0) < 0){
		logMsg(__func__, __LINE__, ERROR, "Error in sending file md5 hash to server. Abort connection.");

		// TODO: free mem and close descriptors
		close(socketDescr);
		free(hashArr_str);
		return EXIT_FAILURE;
	}
	free(hashArr_str);

	// send the file
	logMsg(__func__, __LINE__, INFO, "Start file transferring");
	file_size_t bytes_sended = sendFile(socketDescr, _sendingfile_path);
	if(bytes_sended == -1){
		logMsg(__func__, __LINE__, ERROR, "Error in sending file. Exit.");
		close(socketDescr);
	}
	logMsg(__func__, __LINE__, INFO, "Total send file bytes: %lld.", bytes_sended);

	shutdown(socketDescr, SHUT_WR);
	//close(socketDescr);
	return EXIT_SUCCESS;
}

file_size_t sendFile(int _socket, const char* _full_file_name){
	int fd = 0;
	file_size_t total_bytes_sended = 0;
	file_size_t bytes_sended = 0;
	char* fd_buff = NULL;
	file_size_t bytes_readed = 0;
	int _transfProgress = 0;

	fd_buff = (char*) malloc((SENDING_FILE_PACKET_SIZE) * sizeof(char));
	memset(fd_buff, '\0', (SENDING_FILE_PACKET_SIZE) * sizeof(char));

	fd = open(_full_file_name, O_RDONLY);
	if(fd == -1){
		total_bytes_sended = -1;
	}
	else{
		while( ((bytes_readed = read(fd, fd_buff, SENDING_FILE_PACKET_SIZE * sizeof(char))) > 0) ){

			if( (bytes_sended = send(_socket, fd_buff, bytes_readed, 0)) < 0){
				logMsg(__func__, __LINE__, ERROR, "Error in sending file data block to server. Abort connection.");

				close(fd);
				total_bytes_sended = -1;
				break;
			}

			total_bytes_sended += bytes_sended;

			bytes_sended = 0;
			memset(fd_buff, '\0', SENDING_FILE_PACKET_SIZE);
		}
	}

	free(fd_buff);
	return total_bytes_sended;
}

int initTcpConnSocket(struct sockaddr_in* __tcpsocket_addr, int* __socket_desc, char* __server_addr, int __server_port){

	if((*__socket_desc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return EXIT_FAILURE;

    (*__tcpsocket_addr).sin_family      = AF_INET;
    (*__tcpsocket_addr).sin_addr.s_addr = inet_addr(__server_addr);
    (*__tcpsocket_addr).sin_port        = htons(__server_port);

    return EXIT_SUCCESS;
};
