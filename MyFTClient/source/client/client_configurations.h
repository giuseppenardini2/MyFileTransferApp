/**
 * This header file declares functions, enums and structs for configuring the client.
 *
 * @author peppo02
 */

#ifndef CLIENT_CONFIGURATIONS_H_
#define CLIENT_CONFIGURATIONS_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <file_utils.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

/**
 * An enum that represents the access types.
 */
typedef enum AccessType{
	READ_F=0, /** The 'Read file' access. */
	WRITE_F=1, /** The 'Write file' access. */
	LIST_D=2 /** The 'List directory' access. */
};

/**
 * An enum that represents the message types.
 */
typedef enum MessageType{
	S_OK=0, /** The 'Server OK' message */
	FILE_NOT_FOUND=1, /** The 'File not found' message. */
	DIRECTORY_NOT_FOUND=2, /** The 'Directory not found' message. */
	SPACE_UNAVAILABLE=3 /** The 'Space unavailable' message. */
};

/**
 * A struct that stores client informations.
 */
typedef struct Client{
	size_t id; /** The client identifier. */
	int socket; /** The client socket descriptor. */
	struct sockaddr_in server_address; /** The server address. */
	char *local_path; /** The client path. */
	char *remote_path; /** The server path. */
	enum AccessType access; /** The access type. */
};


struct Client *initialize_client(enum AccessType access, char *server_ip_address, int port,
		char *local_path, char *remote_path);

void free_client(struct Client *client);


#endif /* CLIENT_CONFIGURATIONS_H_ */
