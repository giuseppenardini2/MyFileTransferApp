/**
 * This header file declares functions and structs for executing the server.
 * The server must be initialized with the functions provided by "server_configurations.h"
 * header file.
 *
 * @author peppo02
 */

#ifndef SERVER_EXECUTER_H_
#define SERVER_EXECUTER_H_


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include <file_utils.h>
#include <stdbool.h>
#include <string.h>
#include <sys/file.h>
#include <fcntl.h>
#include <stdint.h>
#include "file_utils.h"
#include "server_configurations.h"


/**
 * A struct that stores client information for the server thread.
 */
typedef struct ClientInfo{
	struct Server *server;
	int client_socket;
};


extern const size_t S_BUFFER_SIZE; /** The server buffer size. */


void server_execute(struct Server *server);

void server_listen(struct Server *server);

void *server_handle_request_thread(void *client_args);

void server_handle_request(struct Server *server, int client_socket);

size_t server_send_id(int client_socket);

void server_receive_access(size_t client_id, int client_socket, enum AccessType *client_access);

void server_receive_remote_path(struct Server *server, size_t client_id, int client_socket, char *remote_path);

bool server_check_request(struct Server *server, size_t client_id, int client_socket, enum AccessType client_access, char *remote_path);

void server_send_message(size_t client_id, int client_socket, enum MessageType message);

void server_receive_message(size_t client_id, int client_socket, enum MessageType *message);

void server_print_message(size_t client_id, char *remote_path, enum MessageType message);

void server_send_ack(size_t client_id, int client_socket);

void server_send_file_size(size_t client_id, int client_socket, char *remote_path);

void server_receive_file_size(size_t client_id, int client_socket, off_t *file_size);

void server_send_file(size_t client_id, int client_socket, char *remote_path);

void server_receive_file(size_t client_id, int client_socket, char *remote_path);

void server_list_directory(size_t client_id, int client_socket, char *remote_path);

void server_shutdown_connection(size_t client_id, int client_socket, enum AccessType client_access);


#endif /* SERVER_EXECUTER_H_ */
