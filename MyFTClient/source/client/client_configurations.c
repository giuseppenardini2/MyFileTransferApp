/**
 * This source file contains implementations of the "client_configurations.h"
 * functions.
 *
 * @author peppo02
 */

#include "client_configurations.h"


/**
 * This function initializes a client.
 *
 * @param access The access type.
 * @param server_ip_address The server ip address.
 * @param port The server port.
 * @param local_path The client path.
 * @param remote_path The server path.
 * @return A pointer to the 'Client' struct.
 */
struct Client *initialize_client(enum AccessType access, char *server_ip_address, int port, char *local_path, char *remote_path){
	printf("CLIENT: Initializing.\n");
	struct Client *client=(struct Client*)malloc(sizeof(struct Client)); //Initialize the 'Client' struct.
	if(client==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT: malloc() error for client: %s (%d).\n", error_message, errno);
		exit(EXIT_FAILURE);
	}

	client->id=0; //Initialize the client id.
	client->access=access; //Set the client access.
	if((access==WRITE_F && local_path==NULL)){ //Check if the necessary path is not null.
		fprintf(stderr, "CLIENT: Error for local_file.\n");
		free(client);
		exit(EXIT_FAILURE);
	}
	else if((access==READ_F || access==LIST_D) && remote_path==NULL){
		fprintf(stderr, "CLIENT: Error for remote_file.\n");
		free(client);
		exit(EXIT_FAILURE);
	}

	if(local_path==NULL){ //Copy the local and remote path into the 'Client' struct.
		client->local_path=strdup(remote_path);
		client->remote_path=strdup(remote_path);
	}
	else if(remote_path==NULL){
		client->remote_path=strdup(local_path);
		client->local_path=strdup(local_path);
	}
	else{
		client->remote_path=strdup(remote_path);
		client->local_path=strdup(local_path);
	}
	if(client->local_path==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT: strdup() for local_path: %s (%d).\n", error_message, errno);
		free(client);
		exit(EXIT_FAILURE);
	}
	if(client->remote_path==NULL){
		char errno_string[256];
		strerror_r(errno, errno_string, sizeof(errno_string));
		fprintf(stderr, "CLIENT: strdup() for remote_path: %s (%d).\n", errno_string, errno);
		free(client);
		exit(EXIT_FAILURE);
	}

	if(client->access==READ_F && check_create_file(client->local_path)==false){ //Check if the local path exists.
		fprintf(stderr, "CLIENT: Invalid local directory or file.\n");
		free_client(client);
		exit(EXIT_FAILURE);
	}
	else if(client->access==WRITE_F && check_file(client->local_path)==false){
		fprintf(stderr, "CLIENT: Invalid local file.\n");
		free_client(client);
		exit(EXIT_FAILURE);
	}

	client->socket=socket(AF_INET, SOCK_STREAM, 0); //Create the client socket.
	if(client->socket<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT: socket() error for client->socket_descriptor: %s (%d).\n", error_message, errno);
		free_client(client);
		exit(EXIT_FAILURE);
	}

	struct timeval timeout; //Set a timeout on recv() to avoid deadlock.
	timeout.tv_sec=5;
	timeout.tv_usec=0;
	if(setsockopt(client->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT: setsockopt() error for client->socket: %s (%d).\n", error_message, errno);
		close(client->socket);
		free_client(client);
		exit(EXIT_FAILURE);
	}

	client->server_address.sin_family=AF_INET; //Initialize the server address of the 'Client' struct.
	client->server_address.sin_port=htons(port);
	if(inet_pton(AF_INET, server_ip_address, &client->server_address.sin_addr)<=0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "CLIENT: inet_pton() error for client->address: %s (%d).\n", error_message, errno);
		close(client->socket);
		free_client(client);
		exit(EXIT_FAILURE);
	}

	printf("CLIENT: Initialized.\n");
	return client;
}

/**
 * This function deallocates the memory reserved for a 'Client' struct and
 * closes the client socket.
 *
 * @param client The 'Client' struct.
 */
void free_client(struct Client *client){
	if(client==NULL)
		return;
	size_t client_id=client->id;
	free(client->local_path);
	free(client->remote_path);
	free(client);
	printf("CLIENT %zu: Free.\n", client_id);
}
