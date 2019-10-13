/*
* server.c
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
#include <math.h>

#define LOOPBACK_ADDR "127.0.0.1"
#define BUF_SIZE 2048
#define FLAGS 0

#define MAX_FILENAME_SIZE 100
#define MAX_FILEDATA_SIZE 1000
#define BINARY_WRITE_MODE "wb"

// Struct that holds information about a fragment
struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char filename[MAX_FILENAME_SIZE];
    char filedata[MAX_FILEDATA_SIZE];
};

// Builds a struct packet from its string representation
// Format => total_frag:frag_no:size:filename:data
// Returns the struct packet
struct packet string_to_packet(char *src) {
    struct packet fragment;
    sscanf(src, "%u:%u:%u:%[^:]", 
            &fragment.total_frag, &fragment.frag_no, 
            &fragment.size, fragment.filename);
    
    // Position in src that filedata starts from
    size_t pos = floor(log10(fragment.total_frag))  // Length of the digits in total_frag
                + floor(log10(fragment.frag_no))    // Length of the digits in frag_no
                + floor(log10(fragment.size))       // Length of the digits in size
                + strlen(fragment.filename)         // Length of filename
                + 7;    // +3 from the 3 integer values as floor(log(x)) gives 1 less than the actual no. of digits in x
                        // +4 from the 4 colons in the format of the string

    char *s = src;
    s += pos;
    for (int i=0; i<fragment.size; ++i) {
        fragment.filedata[i] = *s++;
    }

    return fragment;
}

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

    FILE *file = NULL;
    bool eof = false;   // Flag if eof is received

    while (!eof) {
        // Ready to receive message from client
        num_bytes = recvfrom(socketfd, buf, BUF_SIZE-1, FLAGS, (struct sockaddr *) &client_addr, &client_addr_len);
        if (num_bytes < 0) {
            printf("Error receiving message\n");
            exit(1);
        }

        // Message received. Parse message.
        char ack[10];   // Buffer for acknowledgement

        if (strcmp(buf, "EOF") == 0) {
            // EOF received
            strcpy(ack, "ACK -1");
            eof = true;
        } else {
            // Build packet struct
            struct packet fragment = string_to_packet(buf);
            if (!file) {
                // Create file (overwrite if it exists)
                file = fopen(fragment.filename, BINARY_WRITE_MODE);
                if (!file) {
                    printf("Error opening file\n");
                    close(socketfd);
                    exit(1);
                }
            }
            
            // Append filedata to file
            if (fwrite(fragment.filedata, sizeof(char), fragment.size, file) < 0) {
                printf("Error writing to file\n");
                close(socketfd);
                exit(1);
            }

            // Create acknowledgement
            sprintf(ack, "ACK %d", fragment.frag_no);
        }

        // Send acknowledgement to client
        num_bytes = sendto(socketfd, ack, strlen(ack)+1, FLAGS, (struct sockaddr *) &client_addr, client_addr_len);
        if (num_bytes < 0) {
            printf("Error sending message\n");
            exit(1);
        }
    }

    // Close the file
    if (fclose(file) < 0) {
        printf("Error closing file\n");
        close(socketfd);
        exit(1);
    }

    // Close the socket
    if (close(socketfd) < 0) {
        printf("Error closing socket\n");
        exit(1);
    }
    
    return 0;
}
