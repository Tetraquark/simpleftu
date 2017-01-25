/*
 * Client.c
 *
 *  Created on: 3 янв. 2017 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#include "../include/Client.h"

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
int startClient(char* _serv_ip, int _serv_port, char _sendingfile_path[MAX_FULL_FILE_PATH_LEN + 1], char* _serv_pass){
	struct sockaddr_in tcpsocket_addr_strct;
	socket_t socketDescr = 0;
	size_t outputMsgSize = 0;
	netmsg_stat_code_t status_code;

	// init tcp socket
	if(socket_createPeerTCP(_serv_ip, _serv_port, &tcpsocket_addr_strct, &socketDescr)){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error creating TCP socket. Abort.");
		return EXIT_FAILURE;
	}

	// connect to server
	logMsg(__func__, __LINE__, LOG_INFO, "Try to connect to server: %s:%d", _serv_ip, _serv_port);
	if(connect(socketDescr, (struct sockaddr *) &tcpsocket_addr_strct, sizeof(tcpsocket_addr_strct)) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in connect to server.");
		return EXIT_FAILURE;
	}

	// send password message size
	outputMsgSize = strlen(_serv_pass) * sizeof(char);
	if(socket_sendBytes(socketDescr, &outputMsgSize, sizeof(outputMsgSize)) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in sending message size. Abort connection.");

		// TODO: free mem and close descriptors
		socket_close(socketDescr);
		return EXIT_FAILURE;
	}

	// send password
	logMsg(__func__, __LINE__, LOG_INFO, "Try to send password: %s", _serv_pass);
	if(socket_sendBytes(socketDescr, _serv_pass, outputMsgSize) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in sending password to server. Abort connection.");

		// TODO: free mem and close descriptors
		socket_close(socketDescr);
		return EXIT_FAILURE;
	}

	// recv checking of password result
	if(socket_recvBytes(socketDescr, sizeof(status_code), &status_code) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error receiving answer message from server. Abort connection.");

		socket_close(socketDescr);
		return EXIT_FAILURE;
	}
	if(status_code == INCORRECT){
		logMsg(__func__, __LINE__, LOG_INFO, "Password is incorrect. Abort connection.");

		socket_close(socketDescr);
		return EXIT_SUCCESS;
	}
	else{
		logMsg(__func__, __LINE__, LOG_INFO, "Password is correct.");
	}

	// get the file size
	logMsg(__func__, __LINE__, LOG_INFO, "Counting the sending file size.");
	file_size_t file_size = 0;
	file_size = getFileSize(_sendingfile_path);
	if(file_size == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Cant open file at: %s", _sendingfile_path);

		socket_close(socketDescr);
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
		logMsg(__func__, __LINE__, LOG_ERROR, "Error. Can't get name of file from input path string. Abort.");

		socket_close(socketDescr);
		return EXIT_FAILURE;
	}

	// send file info message size
	char* fileinfo_msg_str = NULL;
	outputMsgSize = serialize_FileInfoMsg(fileinfo_struct, ';', &fileinfo_msg_str);
	if(socket_sendBytes(socketDescr, &outputMsgSize, sizeof(outputMsgSize)) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in sending message size. Abort connection.");

		// TODO: free mem and close descriptors
		socket_close(socketDescr);
		return EXIT_FAILURE;
	}

	// send the file info
	logMsg(__func__, __LINE__, LOG_INFO, "Try to send file info msg: %s", fileinfo_msg_str);
	if(socket_sendBytes(socketDescr, fileinfo_msg_str, outputMsgSize) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error sending file info to server. Abort connection.");

		free(fileinfo_msg_str);

		socket_close(socketDescr);
		return EXIT_FAILURE;
	}
	free(fileinfo_msg_str);


	// recv deserialization file info message result from server
	if(socket_recvBytes(socketDescr, sizeof(status_code), &status_code) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error receiving answer message from server. Abort connection.");

		socket_close(socketDescr);
		return EXIT_FAILURE;
	}
	if(status_code == INCORRECT){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error sending file info. Abort connection.");

		socket_close(socketDescr);
		return EXIT_SUCCESS;
	}
	else{
		logMsg(__func__, __LINE__, LOG_INFO, "The file info was sent.");
	}

	// count the file md5 hash
	logMsg(__func__, __LINE__, LOG_INFO, "Counting the sending file md5 hash.");
	BYTE hashArr[MD5_BLOCK_SIZE];
	memset(hashArr, '\0', MD5_BLOCK_SIZE * sizeof(BYTE));
	if(countFileHash_md5(_sendingfile_path, hashArr)){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in sending file info to server. Abort connection.");

		socket_close(socketDescr);
		return EXIT_FAILURE;
	}

	outputMsgSize = (MD5_BLOCK_SIZE * 2) * sizeof(char);
	char* hashArr_str = (char*) malloc(outputMsgSize);
	memset(hashArr_str, '\0', outputMsgSize);
	fromByteArrToHexStr(hashArr, MD5_BLOCK_SIZE, &hashArr_str);

	// send the file md5 hash message size
	if(socket_sendBytes(socketDescr, &outputMsgSize, sizeof(outputMsgSize)) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in sending message size. Abort connection.");

		// TODO: free mem and close descriptors
		socket_close(socketDescr);
		return EXIT_FAILURE;
	}

	// send the file md5 hash
	logMsg(__func__, __LINE__, LOG_INFO, "Try to send the file md5 hash: %s", hashArr_str);
	if(socket_sendBytes(socketDescr, hashArr_str, outputMsgSize) < 0){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error in sending file md5 hash to server. Abort connection.");

		free(hashArr_str);

		socket_close(socketDescr);
		return EXIT_FAILURE;
	}
	free(hashArr_str);

	// recv file md5 hash sending result
	if(socket_recvBytes(socketDescr, sizeof(status_code), &status_code) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error receiving answer message from server. Abort connection.");

		socket_close(socketDescr);
		return EXIT_FAILURE;
	}
	if(status_code == INCORRECT){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error sending file md5 hash. Abort connection.");

		socket_close(socketDescr);
		return EXIT_SUCCESS;
	}
	logMsg(__func__, __LINE__, LOG_INFO, "File md5 hash was sent.");

	// send the file
	logMsg(__func__, __LINE__, LOG_INFO, "Start file transferring.");
	file_size_t bytes_sended = sendFile(socketDescr, _sendingfile_path);
	if(bytes_sended == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error sending file. Exit.");
		close(socketDescr);
	}
	logMsg(__func__, __LINE__, LOG_INFO, "Total sent bytes: %lld.", bytes_sended);

	// recv result of md5 hashes comparing and transfer result message
	if(socket_recvBytes(socketDescr, sizeof(status_code), &status_code) == -1){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error receiving answer message from server. Abort connection.");

		socket_close(socketDescr);
		return EXIT_FAILURE;
	}
	if(status_code == INCORRECT){
		logMsg(__func__, __LINE__, LOG_ERROR, "Error transferring file. Abort connection.");

		socket_close(socketDescr);
		return EXIT_SUCCESS;
	}
	logMsg(__func__, __LINE__, LOG_INFO, "File successfully transferred.");
#ifdef _WIN32
	shutdown(socketDescr, SD_SEND);
#elif __linux__
	shutdown(socketDescr, SHUT_WR);
#endif
	close(socketDescr);
	return EXIT_SUCCESS;
}

file_size_t sendFile(int _socket, const char* _full_file_name){
	int file_d = 0;
	file_size_t total_bytes_sended = 0;
	file_size_t bytes_sended = 0;
	char* fd_buff = NULL;
	file_size_t bytes_readed = 0;

	fd_buff = (char*) malloc((SENDING_FILE_PACKET_SIZE) * sizeof(char));
	memset(fd_buff, '\0', (SENDING_FILE_PACKET_SIZE) * sizeof(char));

	file_d = open(_full_file_name, O_RDONLY);
	if(file_d == -1){
		total_bytes_sended = -1;
	}
	else{
		while( ((bytes_readed = read(file_d, fd_buff, SENDING_FILE_PACKET_SIZE * sizeof(char))) > 0) ){

			if(socket_sendBytes(_socket, &bytes_readed, sizeof(bytes_readed)) < 0){
				logMsg(__func__, __LINE__, LOG_ERROR, "Data packet send error. Abort connection.");

				close(file_d);
				total_bytes_sended = -1;
				break;
			}

			if( (bytes_sended = socket_sendBytes(_socket, fd_buff, bytes_readed)) < 0){
				logMsg(__func__, __LINE__, LOG_ERROR, "Error send file data block to server. Abort connection.");

				close(file_d);
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
