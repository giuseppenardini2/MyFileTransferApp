/**
 * This header file declares functions and structs for executing the client.
 * The client must be initialized with the functions provided by "client_configurations.h"
 * header file.
 *
 * @author peppo02
 */

#ifndef CLIENT_EXECUTER_H_
#define CLIENT_EXECUTER_H_


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <file_utils.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include "client_configurations.h"


extern const size_t C_BUFFER_SIZE; /** The client buffer size. */


void client_execute(struct Client *client);

void client_connect(struct Client *client);

void client_handle_request(struct Client *client);

void client_receive_id(struct Client *client);

void client_send_access(struct Client *client);

void client_send_remote_path(struct Client *client);

bool client_check_request(struct Client *client);

void client_send_message(struct Client *client, enum MessageType message);

void client_receive_message(struct Client *client, enum MessageType *message);

void client_print_message(struct Client *client, enum MessageType message);

bool client_receive_ack(struct Client *client);

void client_send_file_size(struct Client *client);

void client_receive_file_size(struct Client *client, off_t *file_size);

void client_send_file(struct Client *client);

void client_receive_file(struct Client *client);

void client_list_directory(struct Client *client);


void client_shutdown_connection(struct Client *client);


#endif /* CLIENT_EXECUTER_H_ */
