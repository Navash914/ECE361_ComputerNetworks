/*
* deliver.c
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

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
#include <time.h>

#define BUF_SIZE 1024
#define FLAGS 0
#define MAX_FILEDATA_SIZE 1000
#define BINARY_READ_MODE "rb"

struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char *filename;
    char filedata[MAX_FILEDATA_SIZE];
};

size_t packet_to_string(char *dest, struct packet fragment) {
    // Format => total_frag:frag_no:size:filename:data
    sprintf(dest, "%u:%u:%u:%s:", 
            fragment.total_frag, fragment.frag_no, 
            fragment.size, fragment.filename);
    size_t len = strlen(dest) + fragment.size;
    char *s = dest;
    s += strlen(dest);
    for (int i=0; i<fragment.size; ++i) {
        *s++ = fragment.filedata[i];
    }
    return len;
}

int main(int argc, char **argv) {

    if (argc != 3) {
        // Incorrect usage
        printf("Usage: deliver <server_address> <server_port_number>\n");
        exit(0);
    }

    // Declare Variables
    int socketfd, port, num_bytes;
    char buf[BUF_SIZE], filename[BUF_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t server_addr_len, client_addr_len;
    server_addr_len = sizeof server_addr;
    client_addr_len = sizeof client_addr;

    port = htons(atoi(argv[2]));    // Get port number in network format

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

    // If server address in text form, convert to dots and numbers notation
    inet_aton(argv[1], &server_addr.sin_addr);

    // Get user info for file to send
    printf("Please input a message in the following format: 'ftp <file_name>'\n");
	if (scanf("%s %s", buf, filename) == EOF) {
        printf("Error in input format\n");
        close(socketfd);
        exit(0);
    }

    // Check if file exists
    if(access(filename, F_OK) == -1){
		printf("File does not exist\n");
        close(socketfd);
		exit(0);
	}

    // File exists. Proceed to send packets to server
    clock_t start = clock();

    FILE *file;
    file = fopen(filename, BINARY_READ_MODE);
    if (!file) {
        printf("Error opening file\n");
        close(socketfd);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned int total_frags = filesize / MAX_FILEDATA_SIZE;
    unsigned int last_frag_size = filesize % MAX_FILEDATA_SIZE;
    if (last_frag_size > 0)
        total_frags++;

    struct packet fragments[total_frags];
    int frag_no = 1;
    char buffer[MAX_FILEDATA_SIZE];

    for (int i=0; i<total_frags; ++i) {
        fragments[i].total_frag = total_frags;
        strcpy(fragments[i].filename, filename);
        fragments[i].frag_no = i+1;
        fragments[i].size = fread(fragments[i].filedata, sizeof(char), MAX_FILEDATA_SIZE, file);
    }


    num_bytes = sendto(socketfd, "ftp", strlen("ftp")+1, FLAGS, (struct sockaddr *) &server_addr, server_addr_len);
    if (num_bytes < 0) {
        printf("Error sending message\n");
        close(socketfd);
        exit(1);
    }

    // Receive acknowledgement from server
    num_bytes = recvfrom(socketfd, buf, BUF_SIZE-1, FLAGS, (struct sockaddr *) &server_addr, &server_addr_len);
    if (num_bytes < 0) {
        printf("Error receiving message\n");
        close(socketfd);
        exit(1);
    }


    clock_t end = clock();

    // Time taken for round trip
    double time_elapsed = (double) (end - start) / CLOCKS_PER_SEC;
    time_elapsed *= 1000;

    // Print success message
    if (strcmp(buf, "yes") == 0)
        printf("A file transfer can start.\n");

    // Time elapsed is approximately 0.037ms
    printf("Time taken for round trip: %.3fms\n", time_elapsed);

    // Close the socket
    if (close(socketfd) < 0) {
        printf("Error closing socket\n");
        exit(1);
    }
    
    return 0;
}
