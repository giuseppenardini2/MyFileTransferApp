/**
 * This source file contains implementations of the "server_manager.h"
 * functions.
 *
 * @author peppo02
 */

#include "server_manager.h"


/**
 * This function menages a server.
 *
 * @param ip_address The server ip address.
 * @param port The server port.
 * @param root_directory The server root directory.
 */
void my_FT_server(char *ip_address, int port, char *root_directory){
	struct Server *server=initialize_server(ip_address, port, root_directory); //Initialize the server.
	server_execute(server); //Execute the server.
	free_server(server); //Free the server.
}
