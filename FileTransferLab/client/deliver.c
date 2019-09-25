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

#define BUF_SIZE 1024
#define FLAGS 0

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Usage: deliver <server_address> <server_port_number>\n");
        exit(0);
    }

    int socketfd;
    struct addrinfo hints, *servinfo;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(argv[1], argv[2], &hints, &servinfo) < 0) {
        printf("Error getting server address\n");
        exit(1);
    }

    socketfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (socketfd < 0) {
        printf("Error acquiring socket\n");
        exit(1);
    }

    int num_bytes;
    struct sockaddr_storage server_addr;
    char buf[BUF_SIZE], filename[BUF_SIZE];
    socklen_t addr_len = sizeof server_addr;

    printf("Please input a message in the following format: 'ftp <file_name>\n'");
	if (scanf("%s %s", buf, filename) == EOF) {
        printf("Error in input format\n");
        close(socketfd);
        exit(0);
    }

    if(access(filename, F_OK) == -1){
		printf("File does not exist\n");
        close(socketfd);
		exit(0);
	}

    printf("ready to send\n");

    num_bytes = sendto(socketfd, "ftp", strlen("ftp"), FLAGS, servinfo->ai_addr, servinfo->ai_addrlen);
    if (num_bytes < 0) {
        printf("Error sending message\n");
        close(socketfd);
        exit(1);
    }

    printf("send. waiting to receive\n");

    num_bytes = recvfrom(socketfd, buf, BUF_SIZE-1, FLAGS, (struct sockaddr *) &server_addr, &addr_len);
    if (num_bytes < 0) {
        printf("Error receiving message\n");
        close(socketfd);
        exit(1);
    }

    if (strcmp(buf, "yes") == 0)
        printf("A file transfer can start.\n");

    freeaddrinfo(servinfo);

    if (close(socketfd) < 0) {
        printf("Error closing socket\n");
        exit(1);
    }
    
    return 0;
}