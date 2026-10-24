/**
 * This source file contains the main function.
 *
 * @author peppo02
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "client_configurations.h"
#include "client_manager.h"


int main(){
	test_list();
	return 0;
}

void test_read1(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/IMG1.jpg";
	char remote_path[]="/ClientFiles/IMG1.jpg";
	my_FT_client(READ_F, ip_address, port, local_path, remote_path);
}

void test_read2(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C2/IMG1.jpg";
	char remote_path[]="/ClientFiles/IMG1.jpg";
	my_FT_client(READ_F, ip_address, port, local_path, remote_path);
}

void test_write1(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C3/IMG1.jpg";
	char remote_path[]="/ClientFiles/IMG1.jpg";
	my_FT_client(WRITE_F, ip_address, port, local_path, remote_path);
}

void test_write2(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C4/IMG2.jpg";
	char remote_path[]="/ClientFiles/IMG1.jpg";
	my_FT_client(WRITE_F, ip_address, port, local_path, remote_path);
}

void test_list(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/IMG2.jpg";
	char remote_path[]="/ClientFiles";
	my_FT_client(LIST_D, ip_address, port, local_path, remote_path);
}

void test_read_tf(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/TF9.txt";
	char remote_path[]="/ClientFiles/TF9.txt";
	my_FT_client(READ_F, ip_address, port, local_path, remote_path);
}

void test_write_tf(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/TF9.txt";
	char remote_path[]="/ClientFiles/TF9.txt";
	my_FT_client(WRITE_F, ip_address, port, local_path, remote_path);
}

void test_read_bf(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/BF.zip";
	char remote_path[]="/ClientFiles/BF.zip";
	my_FT_client(READ_F, ip_address, port, local_path, remote_path);
}

void test_write_bf(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/BF.zip";
	char remote_path[]="/ClientFiles/BF.zip";
	my_FT_client(WRITE_F, ip_address, port, local_path, remote_path);
}

void test_read_error(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/TF9.txt";
	char remote_path[]="/CartellaInesistente/TF9.txt";
	my_FT_client(READ_F, ip_address, port, local_path, remote_path);
}

void test_write_error(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/TF9.txt";
	char remote_path[]="/CartellaInesistente/TF9.txt";
	my_FT_client(WRITE_F, ip_address, port, local_path, remote_path);
}

void test_list_error(){
	char ip_address[]="127.0.0.1";
	int port=8080;
	char local_path[]="/home/peppo02/Test/Client/C1/TF9.txt";
	char remote_path[]="/CartellaInesistente/";
	my_FT_client(LIST_D, ip_address, port, local_path, remote_path);
}

void test_client_path(){
	char ip_address[]="127.0.0.1";
	int port=8080;

	char remote_path[]="/home/peppo02/Test/Client/C1/TCP1.txt";
	struct Client *client1=initialize_client(READ_F, ip_address, port, NULL, remote_path);
	printf("CLIENT R: local_path '%s' remote_path '%s'.\n", client1->local_path, client1->remote_path);

	char local_path[]="/home/peppo02/Test/Client/C1/TF9.txt";
	struct Client *client2=initialize_client(WRITE_F, ip_address, port, local_path, NULL);
	printf("CLIENT W: local_path '%s' remote_path '%s'.\n", client2->local_path, client2->remote_path);

	free_client(client1);
	free_client(client2);
}
