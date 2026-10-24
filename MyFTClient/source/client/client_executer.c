/**
 * This source file contains implementations of the "client_executer.h"
 * functions.
 *
 * @author peppo02
 */

#include "client_executer.h"


const size_t C_BUFFER_SIZE=1024;


/**
 * This function executes a client.
 *
 * @param client The 'Client' struct.
 */
void client_execute(struct Client *client){
	printf("CLIENT: Executing.\n");
	client_connect(client); //Connect the client.
	client_handle_request(client); //Handle the client request.
	printf("CLIENT: Execution completed.\n");
}

/**
 * This function connects a client to a server.
 *
 * @param client The 'Client' struct.
 */
void client_connect(struct Client *client){
	printf("CLIENT: Connecting to the server.\n");
	if(connect(client->socket, (struct sockaddr*)&client->server_address, sizeof(client->server_address))<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT: connect() error for client_args: %s (%d).\n", error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
		exit(EXIT_FAILURE);
	}
	printf("CLIENT: Connected to the server.\n");
}

/**
 * This function handles a client request according to the access type.
 *
 * @param client The 'Client' struct.
 */
void client_handle_request(struct Client *client){
	client_receive_id(client); //Receive the id from the server.
	printf("CLIENT %zu: Sending arguments.\n", client->id);
	client_send_access(client); //Send the access type to the server.
	client_send_remote_path(client); //Send the remote path to the server.
	printf("CLIENT %zu: Arguments sent.\n", client->id);

	bool correct_request=client_check_request(client); //Check if the client request is correct.
	if(correct_request==false){
		fprintf(stderr, "CLIENT %zu: Uncorrect request.\n", client->id);
		client_shutdown_connection(client);
		return;
	}

	switch(client->access){ //Execute the client request based on the access type.
		case WRITE_F:
			client_send_file(client);
			break;
		case READ_F:
			client_receive_file(client);
			break;
		case LIST_D:
			client_list_directory(client);
			break;
		default:
			fprintf(stderr, "CLIENT %zu: Unrecognized access\n", client->id);
			break;
	}

	client_shutdown_connection(client); //Close the connection.
}

/**
 * This functions allows a client to receive the identifier from a server.
 *
 * @param client The 'Client' struct.
 */
void client_receive_id(struct Client *client){
	printf("CLIENT: Receiving id.\n");
	uint32_t network_value;
	if(recv(client->socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: recv() error for message: %s (%d).\n", client->id, error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
	}
	client->id=(size_t)ntohl(network_value);
	printf("CLIENT: Id received.\n");
}

/**
 * This function allows a client to send the access type to a server.
 *
 * @param client The 'Client' struct.
 */
void client_send_access(struct Client *client){
	uint32_t network_value=htonl(client->access);
	if(send(client->socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: send() error for message: %s (%d).\n", client->id, error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
		exit(EXIT_FAILURE);
	}
}

/**
 * This function allows a client to send its remote path to a server.
 *
 * @param client The 'Client' struct.
 */
void client_send_remote_path(struct Client *client){
	if(send(client->socket, client->remote_path, strlen(client->remote_path)+1, 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: send() error for client->remote_file: %s (%d).\n", client->id, error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
		exit(EXIT_FAILURE);
	}
}

/**
 * This function checks if a client request was processed correctly by
 * a server.
 *
 * @param client The 'Client' struct.
 * @param If the client request was processed correctly.
 */
bool client_check_request(struct Client *client){
	enum MessageType message=-1;
	switch(client->access){
		case READ_F:
			client_receive_message(client, &message);
			client_print_message(client, message);
			if(message!=S_OK) //Check if the remote path exists.
				return false;
			off_t file_size;
			client_receive_file_size(client, &file_size);
			if(check_available_space(client->local_path, file_size)==false){ //Check if the space is available
				message=SPACE_UNAVAILABLE;
				client_send_message(client, message);
				client_print_message(client, message);
				return false;
			}
			message=S_OK;
			client_send_message(client, message);
			client_print_message(client, message);
			return true;
		case WRITE_F:
			client_receive_ack(client);
			client_send_file_size(client); //Send the file size to check the server available space.
			client_receive_message(client, &message);
			client_print_message(client, message);
			if(message!=S_OK)
				return false;
			client_receive_message(client, &message);
			client_print_message(client, message);
			if(message==S_OK)
				return true;
			return false;
		case LIST_D:
			client_receive_message(client, &message);
			client_print_message(client, message);
			if(message==S_OK)
				return true;
			return false;
		default:
			client_print_message(client, message);
			return false;
	}
}

/**
 * This function allows a client to send a message to a server.
 *
 * @param client The 'Client' struct.
 * @param message The message.
 */
void client_send_message(struct Client *client, enum MessageType message){
	uint32_t network_value=htonl(message);
	if(send(client->socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: send() error for message: %s (%d).\n", client->id, error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
		exit(EXIT_FAILURE);
	}
}

/**
 * This function allows a client to receive a message from a server.
 *
 * @param client The 'Client' struct.
 * @param message The message buffer.
 */
void client_receive_message(struct Client *client, enum MessageType *message){
	uint32_t network_value;
	if(recv(client->socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: recv() error for message: %s (%d).\n", client->id, error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
		exit(EXIT_FAILURE);
	}
	*message=(enum MessageType)ntohl(network_value);
}

/**
 * This function prints a server message.
 *
 * @param client The 'Client' struct.
 * @param message The message to print.
 */
void client_print_message(struct Client *client, enum MessageType message){
	switch(message){
		case S_OK:
			printf("CLIENT %zu: Correct request.\n", client->id);
			break;
		case FILE_NOT_FOUND:
			fprintf(stderr, "CLIENT %zu: File '%s' not found.\n", client->id, client->remote_path);
			break;
		case DIRECTORY_NOT_FOUND:
			fprintf(stderr, "CLIENT %zu: Directory '%s' not found.\n", client->id, client->remote_path);
			break;
		case SPACE_UNAVAILABLE:
			fprintf(stderr, "CLIENT %zu: Space unavailable.\n '%s'.\n", client->id, client->remote_path);
			break;
		default:
			fprintf(stderr, "CLIENT %zu: Unknown message %i.\n", client->id, message);
			break;
	}
}

/**
 * This function allows a client to receive the 'ACK' from a server.
 *
 * @param client The 'Client' struct.
 * @return If the 'ACK' has been received.
 */
bool client_receive_ack(struct Client *client){
	char ack[20];
	if(recv(client->socket, ack, sizeof(ack), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: recv() error for ack: %s (%d).\n", client->id, error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
		exit(EXIT_FAILURE);
	}
	if(strcmp(ack, "ACK")==0)
		return true;
	return false;
}

/**
 * This function allows a client to send the size of its local file to a server.
 *
 * @param client The 'Client' struct.
 */
void client_send_file_size(struct Client *client){
	off_t file_size=get_file_size(client->local_path);
	uint32_t network_value=htonl((uint32_t)file_size);
	if(send(client->socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: send() error for file_size: %s (%d).\n", client->id, error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
		exit(EXIT_FAILURE);
	}
}

/**
 * This function allows a client to receive the size of a remote file from a server
 * and stores it in a buffer.
 *
 * @param client The 'Client' struct.
 * @param file_size The file size buffer.
 */
void client_receive_file_size(struct Client *client, off_t *file_size){
	uint32_t network_value;
	if(recv(client->socket, &network_value, sizeof(network_value), 0)<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: recv() error for message: %s (%d).\n", client->id, error_message, errno);
		client_shutdown_connection(client);
		free_client(client);
		exit(EXIT_FAILURE);
	}
	*file_size=(off_t)ntohl(network_value);
}


/**
 * This function allows a client to send a file to a server.
 *
 * @param client The 'Client' struct.
 */
void client_send_file(struct Client *client){
	printf("CLIENT %zu: Sending file.\n", client->id);
	FILE *file=fopen(client->local_path, "rb"); //Open the file.
	if(file==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: fopen() error for client->local_path '%s': %s (%d).\n", client->id, client->local_path, error_message, errno);
		return;
	}

	char buffer[C_BUFFER_SIZE]; //Send the file to the server.
	size_t bytes_read=0;
	memset(buffer, 0, C_BUFFER_SIZE);
	while((bytes_read=fread(buffer, 1, C_BUFFER_SIZE, file))>0){
		if(send(client->socket, buffer, bytes_read, 0)<0){
			char error_message[256];
			strerror_r(errno, error_message, sizeof(error_message));
			fprintf(stderr, "CLIENT %zu: send() error for client->local_path '%s': %s (%d).\n", client->id, client->local_path, error_message, errno);
		}
		memset(buffer, 0, C_BUFFER_SIZE);
	}

	fclose(file); //Close the file.
	printf("CLIENT %zu: File sent.\n", client->id);
}

/**
 * This function allows a client to receive a file from a server.
 *
 * @param client The 'Client' struct.
 */
void client_receive_file(struct Client *client){
	printf("CLIENT %zu: Receiving file.\n", client->id);
	FILE *file=fopen(client->local_path, "wb"); //Open the file.
	if(file==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT %zu: fopen() error for client->local_path '%s': %s (%d).\n", client->id, client->local_path, error_message, errno);
		return;
	}

	char buffer[C_BUFFER_SIZE]; //Receive the file from the server.
	int bytes_received=0;
	memset(buffer, 0, C_BUFFER_SIZE);
	while((bytes_received=recv(client->socket, buffer, C_BUFFER_SIZE, 0))>0){
		fwrite(buffer, sizeof(char), bytes_received, file);
		fflush(file);
		memset(buffer, 0, C_BUFFER_SIZE);
	}
	if(bytes_received<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT (%zu): recv() error for remote_path '%s': %s (%d).\n", client->id, client->local_path, error_message, errno);
	}

	fclose(file); //Close the file.
	printf("CLIENT %zu: File received.\n", client->id);
}

/**
 * This function allows a client to list server directory.
 *
 * @param client The 'Client' struct.
 */
void client_list_directory(struct Client *client){
	printf("CLIENT %zu: Listing folder.\n", client->id);
	char buffer[C_BUFFER_SIZE];
	int string_length;
	memset(buffer, 0, C_BUFFER_SIZE);
	while((string_length=recv(client->socket, buffer, C_BUFFER_SIZE, 0))>0){ //Receive and print the directory entry of the server.
		printf("CLIENT %zu: Directory content: '%s'.\n", client->id, buffer);
		memset(buffer, 0, C_BUFFER_SIZE);
	}
	if(string_length<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT (%zu): recv() error for remote_path '%s': %s (%d).\n", client->id, client->local_path, error_message, errno);
	}
	printf("CLIENT %zu: Folder listed.\n", client->id);
}

/**
 * This function allows a client to shutdown the connection with a server.
 *
 * @param The 'Client' struct.
 */
void client_shutdown_connection(struct Client *client){
	if(client->access==WRITE_F)
		shutdown(client->socket, SHUT_WR);
	else if(client->access==READ_F || client->access==LIST_D)
		shutdown(client->socket, SHUT_RD);
	close(client->socket);
	printf("CLIENT %zu: Connection closed.\n", client->id);
}
