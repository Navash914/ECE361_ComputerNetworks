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
#include "client_actions.h"

void print_intro() {
    printf("=== Welcome to the Multi-Party Text Conferencing App! ===\n");
    printf("To get started, login to a server with your username and password.\n");
    printf("Available commands:\n  /login <username> <password> <server-ip> <server-port>\n");
    printf("\nYou can use /quit to exit the program anytime\n");
}

int main(int argc, char **argv) {

    if (argc != 1) {
        // Incorrect usage
        printf("Usage: client\n");
        exit(0);
    }

    int socketfd = -1, num_bytes;
    bool valid;
    char buf[BUF_SIZE], *line_buf, command[BUF_SIZE], uname[MAX_NAME];
    size_t buf_size;
    Message msg;

    line_buf = (char *) malloc(BUF_SIZE * sizeof(char) + 1);

    print_intro();

    // Main client loop
    while (true) {
        size_t line_size = getline(&line_buf, &buf_size, stdin);
        if (line_size <= 0)
            continue;
        line_buf[line_size] = '\0';
        strcpy(buf, line_buf);
        sscanf(buf, "%s", command);
        int command_type = parse_client_command(command);

        if (command_type == -1)
            break;  // Quit command

        msg.type = (unsigned int) command_type;
        
        switch (msg.type) {
            case LOGIN:
                if (socketfd > 0) {
                    printf("You are already logged in as %s\n", uname);
                    valid = false;
                } else {
                    socketfd = client_login(buf, &msg);
                    valid = socketfd > 0;
                }
                break;
            case EXIT:
                valid = client_logout(buf, &msg, socketfd);
                break;
            case QUERY:
                valid = client_query(buf, &msg);
                break;
            case NEW_SESS:
                valid = client_create_session(buf, &msg);
                break;
            case JOIN:
                valid = client_join_session(buf, &msg);
                break;
            case MESSAGE:
                valid = client_message(buf, &msg);
                break;
            case LEAVE_SESS:
                valid = client_leave_session(buf, &msg);
                break;
            default:
                valid = false;
                printf("Error parsing client command\n");
                break;
        }

        if (socketfd < 0) {
            valid = false;
            printf("Please /login first before using other commands\n");
        }

        if (valid) {
            if (msg.type == LOGIN) {
                strcpy(uname, msg.source);
            } else {
                strcpy(msg.source, uname);
            }

            msg_to_str(buf, msg);
            num_bytes = send(socketfd, buf, BUF_SIZE-1, FLAGS);
            if (num_bytes < 0) {
                printf("Error sending msg to server\n");
                exit(1);
            }

            if (msg.type == EXIT) {
                printf("Successfully logged out.\n");
                socketfd = -1;
            } else {
                num_bytes = recv(socketfd, buf, BUF_SIZE-1, FLAGS);
                if (num_bytes < 0) {
                    printf("Error receiving msg from server\n");
                    exit(1);
                }
                buf[num_bytes] = '\0';
                msg = str_to_msg(buf);
                client_response(msg);
                if (msg.type == LO_NAK) {
                    close(socketfd);
                    socketfd = -1;
                }
            }
        }
    }

    // Clean up socket if connected
    if (socketfd > 0) {
        if (close(socketfd)) {
            printf("Error closing socket\n");
            exit(1);
        }
    }
    
    return 0;
}
