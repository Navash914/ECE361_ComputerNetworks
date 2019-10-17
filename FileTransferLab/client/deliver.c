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

#define MAX_FILENAME_SIZE 100
#define MAX_FILEDATA_SIZE 1000
#define BINARY_READ_MODE "rb"

// Struct that holds information about a fragment
struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char filename[MAX_FILENAME_SIZE];
    char filedata[MAX_FILEDATA_SIZE];
};

// Converts a fragment struct into a string
// Format => total_frag:frag_no:size:filename:data
// Returns length of string
size_t packet_to_string(char *dest, struct packet fragment) {
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
	
    printf("Time taken for round trip: %.3fms\n", time_elapsed);
	
    // Print message that ftp can start
    if (strcmp(buf, "yes") == 0)
        printf("A file transfer can start.\n");

    // Open file
    FILE *file;
    file = fopen(filename, BINARY_READ_MODE);
    if (!file) {
        printf("Error opening file\n");
        close(socketfd);
        exit(1);
    }

    // Get size of file
    fseek(file, 0, SEEK_END);
    int filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Get total no. of fragments from filesize
    unsigned int total_frags = filesize / MAX_FILEDATA_SIZE;
    if (filesize % MAX_FILEDATA_SIZE > 0)
        total_frags++;

    // Send all fragments to server
    for (int i=0; i<=total_frags; ++i) {
        // Buffer for string representation of packet
        char packet[MAX_FILEDATA_SIZE + strlen(filename) + 100];
        size_t len; // Length of string representation of packet
        int expected_ack_no;    // Should receive this acknowledgement code from server
        if (i == total_frags) {
            // Send EOF packet
            strcpy(packet, "EOF");
            len = 4;
            expected_ack_no = -1;
        } else {
            // Create packet
            struct packet fragment;
            fragment.total_frag = total_frags;
            strcpy(fragment.filename, filename);
            fragment.frag_no = i+1;
            fragment.size = fread(fragment.filedata, sizeof(char), MAX_FILEDATA_SIZE, file);
            len = packet_to_string(packet, fragment);
            expected_ack_no = fragment.frag_no;
        }
        
        // Send packet to server
        num_bytes = sendto(socketfd, packet, len, FLAGS, (struct sockaddr *) &server_addr, server_addr_len);
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

        // ACK Format => ACK <frag_no>
        // frag_no = -1 for EOF Acknowledgement
        char ack[4];
        int ack_no;
        sscanf(buf, "%s %d", ack, &ack_no);
        if (strcmp(ack, "ACK") != 0) {
            printf("Error in ACK format\n");
            close(socketfd);
            exit(1);
        }
        if (ack_no != expected_ack_no) {
            printf("Error in ACK number\n");
            close(socketfd);
            exit(1);
        }
        
    }

    // Print success message
    if (strcmp(buf, "ACK -1") == 0)
        printf("File Transfer Complete.\n");

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
