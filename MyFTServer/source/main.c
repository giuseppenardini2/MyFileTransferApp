/*
 * main.c
 *
 *  Created on: 9 ago 2024
 *      Author: peppo02
 */

#include <file_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "server_configurations.h"
#include "server_manager.h"


int main(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char root_directory[]="/home/peppo02/Test/Server";
	my_FT_server(ip_address, port, root_directory);

	return 0;
}
