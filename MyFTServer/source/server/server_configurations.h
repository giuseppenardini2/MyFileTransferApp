/**
 * This header file declares functions, enums and structs for configuring the server.
 *
 * @author peppo02
 */

#ifndef SERVER_CONFIGURATIONS_H_
#define SERVER_CONFIGURATIONS_H_


#include <stdio.h>
#include <stdlib.h>
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
 * A struct that stores server informations.
 */
typedef struct Server{
	int socket; /** The server socket descriptor. */
	struct sockaddr_in address; /** The server address. */
	char *root_directory; /** The server root directory */
};


extern pthread_mutex_t id_mutex; /** The mutex for the client identifiers. */
extern size_t next_id; /** The next client identifier. */


struct Server *initialize_server(char *ip_address, int port, char *root_directory);

void free_server(struct Server *server);


#endif /* SERVER_CONFIGURATIONS_H_ */
