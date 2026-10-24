/**
 * This source file contains implementations of the "client_manager.h"
 * functions.
 *
 * @author peppo02
 */

#include "client_manager.h"

/**
 * This function menages a client.
 *
 * @param access The access type.
 * @param server_ip_address The server ip address.
 * @param port The server port.
 * @param local_path The client path.
 * @param remote_path The server path.
 */
void my_FT_client(enum AccessType access, char *server_ip_address, int port, char *local_path, char *remote_path){
	struct Client *client=initialize_client(access, server_ip_address, port, local_path, remote_path); //Initialize the client.
	client_execute(client); //Execute the client.
	free_client(client); //Free the client.
}
