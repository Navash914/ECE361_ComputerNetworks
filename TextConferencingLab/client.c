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

typedef struct client_status {
    int socketfd;
    char uname[MAX_NAME];

    bool connected;
    bool exiting;
} ClientStatus;

ClientStatus status;

void receive_msg(ClientStatus *status) {
    char buf[BUF_SIZE];
    while (!status->exiting) {
        while (!status->connected && !status->exiting);
        if (status->exiting)
            break;
        int num_bytes = recv(status->socketfd, buf, BUF_SIZE-1, FLAGS);
        //if (!status->connected)
        //    continue;
        if (num_bytes < 0) {
            printf("Error receiving msg from server\n");
            exit(1);
        }
        buf[num_bytes] = '\0';
        Message msg = str_to_msg(buf);
        client_response(msg);
        if (msg.type == LO_ACK) {
            status->connected = true;
        } else if (msg.type == LO_NAK) {
            status->connected = false;
            close(status->socketfd);
            status->socketfd = -1;
        } else if (msg.type == EXIT) {
            printf("Successfully logged out.\n");
            status->connected = false;
            if (close(status->socketfd)) {
                printf("Error closing socket\n");
                exit(1);
            }
            status->socketfd = -1;
        }
    }
    pthread_exit((void *) 0);
}

int main(int argc, char **argv) {

    if (argc != 1) {
        // Incorrect usage
        printf("Usage: client\n");
        exit(0);
    }

    int num_bytes;
    bool valid;
    char buf[BUF_SIZE], *line_buf, command[BUF_SIZE];
    size_t buf_size;
    Message msg;
    status.socketfd = -1;
    status.connected = false;
    status.exiting = false;

    line_buf = (char *) malloc(BUF_SIZE * sizeof(char) + 1);

    print_intro();
    // Start receive thread
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, (void * (*)(void *)) receive_msg, &status);

    // Main client loop
    while (!status.exiting) {
        size_t line_size = getline(&line_buf, &buf_size, stdin);
        fflush(stdin);
        if (line_size <= 0)
            continue;
        line_buf[line_size] = '\0';
        strcpy(buf, line_buf);
        sscanf(buf, "%s", command);
        int command_type = parse_client_command(command);

        if (command_type == -1) {
            // Quit command
            if (status.connected) {
                // Gracefully logout and let receive thread exit
                command_type = EXIT;

                // Set client status to exiting to exit out on next loop
                status.exiting = true;
            } else break;   // Client not connected. Simply exit.
        }

        if (command_type == HELP) {
            client_help(buf);
            continue;
        }
            

        msg.type = (unsigned int) command_type;
        
        switch (msg.type) {
            case LOGIN:
                if (status.connected) {
                    printf("You are already logged in as %s\n", status.uname);
                    valid = false;
                } else {
                    status.socketfd = client_login(buf, &msg);
                    valid = status.socketfd > 0;
                    status.connected = valid;
                }
                break;
            case EXIT:
                valid = client_logout(buf, &msg);
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
            case MESSAGE_SPEC:
                valid = client_message_specific(buf, &msg);
                break;
            case MESSAGE_ALL:
                valid = client_message_all(buf, &msg);
                break;
            case INVITE:
                valid = client_invite(buf, &msg);
                break;
            case INVITE_LIST:
                valid = client_invite_list(buf, &msg);
                break;
            case INVITE_RESP:
                valid = client_invite_response(buf, &msg);
                break;
            case LEAVE_SESS:
                valid = client_leave_session(buf, &msg);
                break;
            default:
                valid = false;
                printf("Error parsing client command\n");
                break;
        }

        if (!status.connected) {
            valid = false;
            printf("Please /login first before using other commands\n");
        }

        if (valid) {
            if (msg.type == LOGIN) {
                strcpy(status.uname, msg.source);
            } else {
                strcpy(msg.source, status.uname);
            }

            msg_to_str(buf, msg);
            num_bytes = send(status.socketfd, buf, BUF_SIZE-1, FLAGS);
            if (num_bytes < 0) {
                printf("Error sending msg to server\n");
                exit(1);
            }
        }
    }

    status.connected = false;
    status.exiting = true;
    pthread_join(receive_thread, NULL);

    // Clean up socket if connected
    if (status.socketfd > 0) {
        if (close(status.socketfd)) {
            printf("Error closing socket\n");
            exit(1);
        }
    }
    
    return 0;
}
