/*
* client.c
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
#include <errno.h>

#include "defines.h"
#include "message.h"

#define CC_TEXT 0
#define CC_LOGIN 1
#define CC_JNSESS 2
#define CC_LVSESS 3
#define CC_CRSESS 4
#define CC_LIST 5
#define CC_QUIT 6

int parse_client_command(char *command) {
    if (command[0] != '/')
        return CC_TEXT;
    if (!strcmp(command, "/login"))
        return CC_LOGIN;
    else if (!strcmp(command, "/logout"))
        return CC_LOGOUT;
    else if (!strcmp(command, "/joinsession"))
        return CC_JNSESS;
    else if (!strcmp(command, "/leavesession"))
        return CC_LVSESS;
    else if (!strcmp(command, "/createsession"))
        return CC_CRSESS;
    else if (!strcmp(command, "/list"))
        return CC_LIST;
    else if (!strcmp(command, "/quit"))
        return CC_LIST;
    else
        return CC_QUIT;
}

int main(int argc, char **argv) {

    if (argc != 1) {
        // Incorrect usage
        printf("Usage: client\n");
        exit(0);
    }

    bool quit = false;
    char line_buf[BUF_SIZE], command[BUF_SIZE];
    size_t line_buf_size;

    // Take command from user
    while (!quit) {
        size_t line_size = getline(&line_buf, &line_buf_size, stdin);
        if (line_size <= 0)
            continue;
        sscanf(line_buf, "%s", command);
        int command_type = parse_client_command(command);
        if (command_type == CC_TEXT)
            send_text_message(line_buf);
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

    // Set timeout option for socket
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = TIMEOUT_MSEC;
    if (setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        printf("Failed to config receive timeout\n");
        close(socketfd);
        exit(1);
    }
	
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
            if (errno == EAGAIN) {
                // Timeout occurred
                printf("Timeout for ACK. Resending fragment #%d\n", i+1);
                // Resend packet
                i--;
                continue;
            }
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
