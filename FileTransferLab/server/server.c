#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define LOOPBACK_ADDR "127.0.0.1"
#define BUF_SIZE 1024
#define FLAGS 0

int main(int argc, char **argv) {

    if (argc != 2) {
        // Incorrect usage
        printf("Usage: socket <port_number>\n");
        exit(0);
    }

    // Declare variables
    int socketfd, port, num_bytes;
    char buf[BUF_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t server_addr_len, client_addr_len;
    server_addr_len = sizeof server_addr;
    client_addr_len = sizeof client_addr;

    port = htons(atoi(argv[1]));    // Get port number in network format

    // Get socket
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
        printf("Error acquiring socket\n");
        exit(1);
    }

    // Create the address struct for server
    memset(&server_addr, 0, server_addr_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(socketfd, (struct sockaddr *) &server_addr, server_addr_len) < 0) {
        printf("Error binding to socket\n");
        exit(1);
    }

    // Ready to receive message from client
    num_bytes = recvfrom(socketfd, buf, BUF_SIZE-1, FLAGS, (struct sockaddr *) &client_addr, &client_addr_len);
    if (num_bytes < 0) {
        printf("Error receiving message\n");
        exit(1);
    }

    // Message received. Parse message.
    char msg[4];

    if (strcmp(buf, "ftp") == 0)
        strcpy(msg, "yes\0");
    else
        strcpy(msg, "no\n");

    // Send acknowledgement to client
    num_bytes = sendto(socketfd, msg, strlen(msg), FLAGS, (struct sockaddr *) &client_addr, client_addr_len);
    if (num_bytes < 0) {
        printf("Error sending message\n");
        exit(1);
    }

    // Close the socket
    if (close(socketfd) < 0) {
        printf("Error closing socket\n");
        exit(1);
    }
    
    return 0;
}