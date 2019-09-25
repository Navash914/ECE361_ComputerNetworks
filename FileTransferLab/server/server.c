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

    int socketfd;
    struct addrinfo hints, *servinfo;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(LOOPBACK_ADDR, argv[1], &hints, &servinfo) < 0) {
        printf("Error getting server address\n");
        exit(1);
    }

    socketfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (socketfd < 0) {
        printf("Error acquiring socket\n");
        exit(1);
    }

    if (bind(socketfd, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
        printf("Error binding to socket\n");
        exit(1);
    }

    freeaddrinfo(servinfo);

    int num_bytes;
    struct sockaddr_storage client_addr;
    char buf[BUF_SIZE];
    socklen_t addr_len = sizeof client_addr;

    printf("ready to receive\n");

    num_bytes = recvfrom(socketfd, buf, BUF_SIZE-1, FLAGS, (struct sockaddr *) &client_addr, &addr_len);
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

    num_bytes = sendto(socketfd, msg, strlen(msg), FLAGS, (struct sockaddr *) &client_addr, addr_len);
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