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

    int socketfd, port, num_bytes;
    char buf[BUF_SIZE], filename[BUF_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t server_addr_len, client_addr_len;
    server_addr_len = sizeof server_addr;
    client_addr_len = sizeof client_addr;

    port = htons(atoi(argv[2]));

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
        printf("Error acquiring socket\n");
        exit(1);
    }

    memset(&server_addr, 0, server_addr_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    inet_aton(argv[1], &server_addr.sin_addr);

    printf("Please input a message in the following format: 'ftp <file_name>'\n");
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

    num_bytes = sendto(socketfd, "ftp", strlen("ftp")+1, FLAGS, (struct sockaddr *) &server_addr, server_addr_len);
    if (num_bytes < 0) {
        printf("Error sending message\n");
        close(socketfd);
        exit(1);
    }

    num_bytes = recvfrom(socketfd, buf, BUF_SIZE-1, FLAGS, (struct sockaddr *) &server_addr, &server_addr_len);
    if (num_bytes < 0) {
        printf("Error receiving message\n");
        close(socketfd);
        exit(1);
    }

    if (strcmp(buf, "yes") == 0)
        printf("A file transfer can start.\n");

    if (close(socketfd) < 0) {
        printf("Error closing socket\n");
        exit(1);
    }
    
    return 0;
}