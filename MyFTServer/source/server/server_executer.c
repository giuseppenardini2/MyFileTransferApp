/**
 * This source file contains implementations of the "server_executer.h"
 * functions.
 *
 * @author peppo02
 */

#include "server_executer.h"


const size_t S_BUFFER_SIZE=1024;


/**
 * This function executes a server.
 *
 * @param server The 'Server' struct.
 */
void server_execute(struct Server *server){
	server_listen(server); //Listen.

	printf("SERVER: Executing.\n");
	while(true){
		struct sockaddr_in client_address;
		socklen_t client_length=sizeof(client_address);
		int client_socket=accept(server->socket, (struct sockaddr*)&client_address, &client_length); //Accept a client.
		if(client_socket<0)
			continue;
		struct ClientInfo *client_info=(struct ClientInfo*)malloc(sizeof(struct ClientInfo)); //Create a server thread to handle the client accepted.
		client_info->server=server;
		client_info->client_socket=client_socket;
		pthread_t server_tid;
		if(pthread_create(&server_tid, NULL, server_handle_request_thread, client_info)!=0){
			char error_message[256];
			strerror_r(errno, error_message, sizeof(error_message));
			fprintf(stderr, "SERVER: pthread_create() error for server_tid: %s (%d).\n", error_message, errno);
			free(client_info);
			exit(EXIT_FAILURE);
		}
		pthread_detach(server_tid);
	}
	printf("SERVER: Execution finished.\n");
}

/**
 * This function allows a server to listen.
 *
 * @param server The 'Server' struct.
 */
void server_listen(struct Server *server){
	if(listen(server->socket, 10)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER: listen() error for server_args: %s (%d).\n", error_message, errno);
		exit(EXIT_FAILURE);
	}
	printf("SERVER: Listening.\n");
}

/**
 * This function handles a client request with a thread.
 *
 * @param client_args The 'ClientInfo' struct.
 */
void *server_handle_request_thread(void *client_args){
	struct ClientInfo *client_info=(struct ClientInfo*)client_args;
	server_handle_request(client_info->server, client_info->client_socket);
	pthread_exit(NULL);
}

/**
 * This function handles a client request.
 *
 * @param server The 'Server' struct.
 * @param client_socket The client socket.
 */
void server_handle_request(struct Server *server, int client_socket){
	size_t client_id;
	client_id=server_send_id(client_socket); //Send the id to the client.
	printf("SERVER: Receiving arguments from a client %zu.\n", client_id);
	enum AccessType client_access;
	server_receive_access(client_id, client_socket, &client_access); //Receive the access type from the client.
	char remote_path[S_BUFFER_SIZE];
	server_receive_remote_path(server, client_id, client_socket, remote_path); //Receive the remote path from the client.
	printf("SERVER: Arguments receive from client %zu.\n", client_id);

	bool correct_request=server_check_request(server, client_id, client_socket, client_access, remote_path); //Check the client request.
	if(correct_request==false){
		fprintf(stderr, "SERVER: Uncorrect client %zu request.\n", client_id);
		server_shutdown_connection(client_id, client_socket, client_access);
		return;
	}

	switch(client_access){ //Execute the client request based on the access type.
		case WRITE_F:
			server_receive_file(client_id, client_socket, remote_path);
			break;
		case READ_F:
			server_send_file(client_id, client_socket, remote_path);
			break;
		case LIST_D:
			server_list_directory(client_id, client_socket, remote_path);
			break;
		default:
			fprintf(stderr, "SERVER: Unrecognized access %i\n", client_access);
			break;
	}

	server_shutdown_connection(client_id, client_socket, client_access); //Close the connection.
	printf("SERVER: Client %zu served.\n", client_id);
}

/**
 * This function creates a client identifier and allows a server to send it to
 * a client.
 *
 * @param client_socket The client socket.
 * @return The client identifier.
 */
size_t server_send_id(int client_socket){
	printf("SERVER: Sending id.\n");
	size_t client_id; //Create the id.
	pthread_mutex_lock(&id_mutex);
	client_id=next_id;
	next_id++;
	pthread_mutex_unlock(&id_mutex);
	uint32_t network_value=htonl((uint32_t)client_id);

	if(send(client_socket, &network_value, sizeof(network_value), 0)<0){ //Send the id to the client.
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): send() error for client_id: %s (%d).\n", client_id, error_message, errno);
		server_shutdown_connection(client_id, client_socket, -1);
	}
	printf("SERVER: Id sent to client %zu.\n", client_id);
	return client_id;
}

/**
 * This functions allows a server to receive the access type from a client and stores
 * it in a buffer.
 *
 * @param client_socket The client socket.
 * @param client_access The access type buffer.
 */
void server_receive_access(size_t client_id, int client_socket, enum AccessType *client_access){
	uint32_t network_value;
	if(recv(client_socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): recv() error for message: %s (%d).\n", client_id, error_message, errno);
		server_shutdown_connection(client_id, client_socket, -1);
	}
	*client_access=(enum MessageType)ntohl(network_value);
}

/**
 * This function allows a server to receive the remote path from a client
 * and stores it in a buffer.
 * The remote path is concateneted with the server root directory.
 *
 * @param server The 'Server' struct.
 * @param client_socket The client socket.
 * @param remote_path The remote path buffer.
 */
void server_receive_remote_path(struct Server *server, size_t client_id, int client_socket, char *remote_path){
	char client_remote_path[S_BUFFER_SIZE];
	if(recv(client_socket, &client_remote_path, sizeof(client_remote_path), 0)<=0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): recv() error for client->remote_path: %s (%d).\n", client_id, error_message, errno);
		server_shutdown_connection(client_id, client_socket, -1);
		return;
	}
	strcpy(remote_path, server->root_directory);
	strcat(remote_path, client_remote_path);
}

/**
 * This function checks if a client request was processed correctly.
 *
 * @param server The 'Server' struct.
 * @param client_id The client identifier.
 * @param client_access The access type.
 * @param remote_path The remote path.
 */
bool server_check_request(struct Server *server, size_t client_id, int client_socket, enum AccessType client_access, char *remote_path){
	enum MessageType message=-1;
	switch(client_access){
	case READ_F:
		if(check_file(remote_path)==false){ //Check if the file exists.
			message=FILE_NOT_FOUND;
			server_send_message(client_id, client_socket, message);
			server_print_message(client_id, remote_path, message);
			return false;
		}
		message=S_OK;
		server_send_message(client_id, client_socket, message);
		server_print_message(client_id, remote_path, message);
		server_send_file_size(client_id, client_socket, remote_path); //Send the file size to check the client available space.
		server_receive_message(client_id, client_socket, &message);
		server_print_message(client_id, remote_path, message);
		if(message==S_OK)
			return true;
		return false;
	case WRITE_F:
		server_send_ack(client_id, client_socket);
		off_t file_size;
		server_receive_file_size(client_id, client_socket, &file_size);
		if(check_available_space(server->root_directory, file_size)==false){ //Check if the space is available for the file.
			message=SPACE_UNAVAILABLE;
			server_send_message(client_id, client_socket, message);
			server_print_message(client_id, remote_path, message);
			return false;
		}
		message=S_OK;
		server_send_message(client_id, client_socket, message);
		server_print_message(client_id, remote_path, message);
		if(check_create_file(remote_path)==false){ //Check if the file exists, else create it.
			message=FILE_NOT_FOUND;
			server_send_message(client_id, client_socket, message);
			server_print_message(client_id, remote_path, message);
			return false;
		}
		message=S_OK;
		server_send_message(client_id, client_socket, message);
		server_print_message(client_id, remote_path, message);
		return true;
	case LIST_D:
		if(check_directory(remote_path)==false){ //Check if the directory exists.
			message=DIRECTORY_NOT_FOUND;
			server_send_message(client_id, client_socket, message);
			server_print_message(client_id, remote_path, message);
			return false;
		}
		message=S_OK;
		server_send_message(client_id, client_socket, message);
		server_print_message(client_id, remote_path, message);
		return true;
	default:
		fprintf(stderr, "SERVER: Unrecognized access %i.\n", client_access);
		return false;
	}
}

/**
 * This function allows a server to send a message to a client.
 *
 * @param client_id The client identifier.
 * @param client_socket The client socket.
 * @param message The server message.
 */
void server_send_message(size_t client_id, int client_socket, enum MessageType message){
	int network_value=htonl(message);
	if(send(client_socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): send() error for message: %s (%d).\n", client_id, error_message, errno);
		server_shutdown_connection(client_id, client_socket, -1);
	}
}

/**
 * This function allows a server to receive a message from a client.
 *
 * @param client_id The client identifier.
 * @param client_socket The client socket.
 * @param message The message buffer.
 */
void server_receive_message(size_t client_id, int client_socket, enum MessageType *message){
	uint32_t network_value;
	if(recv(client_socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): recv() error for message: %s (%d).\n", client_id, error_message, errno);
		server_shutdown_connection(client_id, client_socket, -1);
	}
	*message=(enum MessageType)ntohl(network_value);
}

/**
 * This functions prints a server message.
 *
 * @param client_id The client identifier.
 * @param remote_path The remote path.
 * @param message The message to print.
 */
void server_print_message(size_t client_id, char *remote_path, enum MessageType message){
	switch(message){
		case S_OK:
			printf("SERVER: Correct request for client %zu.\n", client_id);
			break;
		case FILE_NOT_FOUND:
			fprintf(stderr, "SERVER (%zu): File '%s' not found.\n", client_id, remote_path);
			break;
		case DIRECTORY_NOT_FOUND:
			fprintf(stderr, "SERVER (%zu): Directory '%s' not found.\n", client_id,remote_path);
			break;
		case SPACE_UNAVAILABLE:
			fprintf(stderr, "SERVER (%zu): Space unavailable.\n '%s'.\n", client_id, remote_path);
			break;
		default:
			fprintf(stderr, "SERVER (%zu): Unknown message %i.\n", client_id, message);
			break;
	}
}

/**
 * This function allows a server to send the 'ACK' to a client.
 *
 * @param client_id The client identifier.
 * @param client_socket The client socket.
 */
void server_send_ack(size_t client_id, int client_socket){
	char ack[]="ACK";
	if(send(client_socket, ack, sizeof(ack), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): send() error for ack: %s (%d).\n", client_id, error_message, errno);
		server_shutdown_connection(client_id, client_socket, -1);
	}
}

/**
 * This functions allows a server to send its file size to a client.
 *
 * @param client_id The client identifier.
 * @param client_socket The client socket.
 * @param remote_path The file path.
 */
void server_send_file_size(size_t client_id, int client_socket, char *remote_path){
	off_t file_size=get_file_size(remote_path);
	uint32_t network_value=htonl((uint32_t)file_size);
	if(send(client_socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): send() error for file_size: %s (%d).\n", client_id, error_message, errno);
		server_shutdown_connection(client_id, client_socket, -1);
	}
}

/**
 * This function allows a server to receive the size of a local file from a client
 * and stores it in a buffer.
 *
 * @param client_id The client identifier.
 * @param client_socket The client socket.
 * @param file_size The file size buffer.
 */
void server_receive_file_size(size_t client_id, int client_socket, off_t *file_size){
	uint32_t network_value;
	if(recv(client_socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): recv() error for message: %s (%d).\n", client_id, error_message, errno);
		server_shutdown_connection(client_id, client_socket, -1);
	}
	*file_size=(off_t)ntohl(network_value);
}


/**
 * This function allows a server to send a file to a client.
 *
 * @param client_id The client id.
 * @param client_socket The client socket.
 * @param remote_path The file path.
 */
void server_send_file(size_t client_id, int client_socket, char *remote_path){
	printf("SERVER: Sending file to the client %zu.\n", client_id);
	FILE *file=fopen(remote_path, "rb"); //Open the file.
	if(file==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): fopen() error for remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
		return;
	}

	int file_descriptor=fileno(file); //Lock the file to avoid race condition.
	if(flock(file_descriptor, LOCK_SH)==-1){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): flock() error while locking remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
	}

	char buffer[S_BUFFER_SIZE]; //Send the file to the client.
	size_t bytes_read=0;
	memset(buffer, 0, S_BUFFER_SIZE);
	while((bytes_read=fread(buffer, sizeof(char), S_BUFFER_SIZE, file))>0){
		if(send(client_socket, buffer, bytes_read, 0)<0){
			char error_message[256];
			strerror_r(errno, error_message, sizeof(error_message));
			fprintf(stderr, "SERVER (%zu): send() error for remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
		}
		memset(buffer, 0, S_BUFFER_SIZE);
	}

	if(flock(file_descriptor, LOCK_UN)==-1){ //Unlock the file.
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): flock() error while unlocking remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
	}

	fclose(file); //Close the file.
	printf("SERVER: File sent to the client %zu.\n", client_id);
}

/**
 * This function allows a server to receive a file from a client.
 *
 * @param client_id The client id.
 * @param client_socket The client socket.
 * @param remote_path The file path.
 */
void server_receive_file(size_t client_id, int client_socket, char *remote_path){
	printf("SERVER: Receiving file from client %zu.\n", client_id);

	FILE *file=fopen(remote_path, "wb"); //Open the file.
	if(file==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): fopen() error for remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
		return;
	}

	int file_descriptor=fileno(file); //Lock the file to avoid race condition.
	if(flock(file_descriptor, LOCK_EX)==-1){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): flock() error while locking remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
	}

	char buffer[S_BUFFER_SIZE]; //Receive the file from the client.
	int bytes_received=0;
	memset(buffer, 0, S_BUFFER_SIZE);
	while((bytes_received=recv(client_socket, buffer, S_BUFFER_SIZE, 0))>0){
		fwrite(buffer, sizeof(char), bytes_received, file);
		fflush(file);
		memset(buffer, 0, S_BUFFER_SIZE);
	}
	if(bytes_received<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): recv() error for remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
	}

	if(flock(file_descriptor, LOCK_UN)==-1){ //Unlock the file.
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): flock() error while unlocking remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
	}

	fclose(file); //Close the file.
	printf("SERVER: File received from client %zu.\n", client_id);
}

/**
 * This function allows a server to list its directory.
 *
 * @param client_id The client id.
 * @param client_socket The client socket.
 * @param remote_path The directory path.
 */
void server_list_directory(size_t client_id, int client_socket, char *remote_path){
	printf("SERVER: Listing directory for client %zu.\n", client_id);

	DIR *directory=opendir(remote_path); //Open the directory.
	if(directory==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER (%zu): opendir() error for remote_path '%s': %s (%d).\n", client_id, remote_path, error_message, errno);
		return;
	}

	char buffer[S_BUFFER_SIZE]; //Read the directory entries.
	struct dirent *directory_entry;
	memset(buffer, 0, S_BUFFER_SIZE);
	while((directory_entry=readdir(directory))!=NULL){
		strcpy(buffer, directory_entry->d_name);
		if(send(client_socket, &buffer, S_BUFFER_SIZE, 0)<0){ //Send the directory entries to the client.
			char error_message[256];
			strerror_r(errno, error_message, sizeof(error_message));
			fprintf(stderr, "SERVER (%zu): send() error for directory_entry: %s (%d).\n", client_id, error_message, errno);
			break;
		}
		memset(buffer, 0, S_BUFFER_SIZE);
	}

	closedir(directory); //Close the directory.
	printf("SERVER: Directory listed for client %zu.\n", client_id);
}

/**
 * This function allows a server to shutdown the connection with a client.
 *
 * @param client_id The client id.
 * @param client_socket The client socket.
 * @param client_access The client access.
 */
void server_shutdown_connection(size_t client_id, int client_socket, enum AccessType client_access){
	if(client_access==WRITE_F)
		shutdown(client_socket, SHUT_RD);
	else if(client_access==READ_F || client_access==LIST_D)
		shutdown(client_socket, SHUT_WR);
	close(client_socket);
	printf("SERVER: Connection closed with client %zu.\n", client_id);
}
