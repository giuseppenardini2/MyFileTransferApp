/**
 * This header file declares functions for managing the client.
 *
 * @author peppo02
 */

#ifndef CLIENT_MANAGER_H_
#define CLIENT_MANAGER_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "client_configurations.h"
#include "client_executer.h"


void my_FT_client(enum AccessType access, char *server_ip_address, int port, char *local_path, char *remote_path);


#endif /* CLIENT_MANAGER_H_ */
