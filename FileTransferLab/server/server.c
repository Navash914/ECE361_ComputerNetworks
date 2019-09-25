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
        printf("Usage: socket <port_number>\n");
        exit(0);
    }

    int socketfd, port, num_bytes;
    char buf[BUF_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t server_addr_len, client_addr_len;
    server_addr_len = sizeof server_addr;
    client_addr_len = sizeof client_addr;

    port = htons(atoi(argv[1]));

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
        printf("Error acquiring socket\n");
        exit(1);
    }

    memset(&server_addr, 0, server_addr_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socketfd, (struct sockaddr *) &server_addr, server_addr_len) < 0) {
        printf("Error binding to socket\n");
        exit(1);
    }

    printf("ready to receive\n");

    num_bytes = recvfrom(socketfd, buf, BUF_SIZE-1, FLAGS, (struct sockaddr *) &client_addr, &client_addr_len);
    if (num_bytes < 0) {
        printf("Error receiving message\n");
        exit(1);
    }

    printf("received, ready to send\n");

    char msg[4];

    if (strcmp(buf, "ftp") == 0)
        strcpy(msg, "yes\0");
    else
        strcpy(msg, "no\n");

    num_bytes = sendto(socketfd, msg, strlen(msg), FLAGS, (struct sockaddr *) &client_addr, client_addr_len);
    if (num_bytes < 0) {
        printf("Error sending message\n");
        exit(1);
    }

    if (close(socketfd) < 0) {
        printf("Error closing socket\n");
        exit(1);
    }
    
    return 0;
}