/**
 * This source file contains implementations of the "server_configurations.h"
 * functions.
 *
 * @author peppo02
 */

#include "server_configurations.h"


pthread_mutex_t id_mutex;
size_t next_id=1;


/**
 * This function initializes a 'Server' struct.
 *
 * @param ip_address The server ip address.
 * @param port The server port.
 * @param root_directory The server root directory.
 * @return A pointer to the 'Server' struct.
 */
struct Server *initialize_server(char *ip_address, int port, char *root_directory){
	pthread_mutex_init(&id_mutex, NULL); //Initialize the identifiers mutex.

	printf("SERVER: initializing.\n");
	struct Server *server=(struct Server*)malloc(sizeof(struct Server)); //Create a 'Server' struct.
	if(server==NULL){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER: malloc() error for server: %s (%d).\n", error_message, errno);
		exit(EXIT_FAILURE);
	}

	if(check_create_directory(root_directory)==false){
		fprintf(stderr, "SERVER: Invalid root_directory.\n");
		exit(EXIT_FAILURE);
	}
	server->root_directory=strdup(root_directory);

	server->socket=socket(AF_INET, SOCK_STREAM, 0); //Create the server socket.
	if(server->socket<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER: socket() error for server->socket_descriptor: %s (%d).\n", error_message, errno);
		free_server(server);
		exit(EXIT_FAILURE);
	}

	struct timeval timeout; //Set a timeout on recv() to avoid deadlock.
	timeout.tv_sec=5;
	timeout.tv_usec=0;
	if(setsockopt(server->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))<0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER: setsockopt() error for server->socket: %s (%d).\n", error_message, errno);
		free_server(server);
		exit(EXIT_FAILURE);
	}

	server->address.sin_family=AF_INET; //Set the server address of the 'Server' struct.
	server->address.sin_port=htons(port);
	if(inet_pton(AF_INET, ip_address, &server->address.sin_addr)<=0){
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER: inet_pton() error for server->address: %s (%d).\n", error_message, errno);
		free_server(server);
		exit(EXIT_FAILURE);
	}

	if(bind(server->socket, (struct sockaddr*)&server->address, sizeof(server->address))<0){ //Assign the server address to the server socket.
		char error_message[256];
		strerror_r(errno, error_message, sizeof(error_message));
		fprintf(stderr, "SERVER: bind() error for server: %s (%d).\n", error_message, errno);
		free_server(server);
		exit(EXIT_FAILURE);
	}

	printf("SERVER: Initialized.\n");
	return server;
}

/**
 * This functions deallocates the memory reserved for a 'Server' struct.
 *
 * @param server The 'Server' struct.
 */
void free_server(struct Server *server){
	pthread_mutex_destroy(&id_mutex); //Destroy the identifiers mutex.
	if(server==NULL)
		return;
	close(server->socket);
	free(server->root_directory);
	free(server);
	printf("SERVER: Free.\n");
}
