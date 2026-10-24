# MyFileTransferApp

## Table of contents
- [About](#about)
- [Configuration](#configuration)
- [Execution](#Execution)
- [License](#license)

## About
MyFileTransferApp is an application that allows multiple clients to exchange files with a server.
The program was designed using a modular programming style to promote
scalability, readability, reusability, and maintainability of the code.
It is structured into two subprograms representing the client and the server, and both are
divided into two main parts¹: client/server configuration and execution.

> [!WARNING]  
> MyFileTransferApp is a university project focused on a client-server file transfer.
> The security features are overlooked in order to focus on the rest.

## Configuration

### Server Configuration
The server configuration is managed through a module (server_configurations.h) that
sets its network address, creates its socket, and associates the network address with the
socket.

### Client Configuration
Client configuration is managed through a module (client_configurations.h) that
sets the client’s network address, creates the socket, and verifies that the passed paths are
correct.

## Execution

### Server Execution
Server execution is handled by a module (server_executer.h) that is responsible2 for
setting the server to listen, accepting client connections3, receiving arguments from clients,
verifying their requests, executing them4, and closing the connection with them.

### Client Execution
Client execution is handled by a module (client_executer.h) that is responsible5 for
connecting to the server, sending their arguments to the server, executing their request, and closing their
connection.

## License
*Copyright© 2026 Giuseppe Nardini*\
This program is licensed under the [AGPL-3.0](LICENSE) license.
