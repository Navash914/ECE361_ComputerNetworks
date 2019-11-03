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
#include <pthread.h>

#include "message.h"

#define LOOPBACK_ADDR "127.0.0.1"
#define BACKLOG 10

// Subroutine for each client
void client_subroutine(User *user) {
    int num_bytes;
    Message msg_send, msg_recv;
    char buf[BUF_SIZE];
    bool exiting = false;

    while (true) {
        num_bytes = recv(user->sockfd, buf, BUF_SIZE-1, FLAGS);
        if (num_bytes < 0) {
            printf("Error receiving message from client\n");
            exit(1);
        }
        if (num_bytes == 0) {
            if (user->logged_in)
                printf("%s closed connection\n");
            else
                printf("Unknown user closed connection\n");
            exiting = true;
        }
        buf[num_bytes] = '\0';

        if (exiting)
            break;

        msg_recv = str_to_msg(buf);
        switch (msg_recv.type) {
            case LOGIN:
                msg_send = server_login(user, msg_recv);
                break;
            case EXIT:
                exiting = true;
                break;
            case QUERY:
                msg_send = server_query(user, msg_recv);
                break;
            case NEW_SESS:
                msg_send = server_create_session(user, msg_recv);
                break;
            case JOIN_SESS:
                msg_send = server_join_session(user, msg_recv);
                break;
            case MESSAGE:
                msg_send = server_message(user, msg_recv);
                break;
            case LEAVE_SESS:
                msg_send = server_leave_session(user, msg_recv);
                break;
            default:
                printf("Invalid message type\n");
                break;
        }

        if (exiting)
            break;

        msg_to_str(buf, msg_send);
        num_bytes = send(user->sockfd, buf, BUF_SIZE-1, FLAGS);
        if (num_bytes < 0) {
            printf("Error sending message to client\n");
        }
    }

    // Client exit
    strcpy(buf, user->username);
    if (user->logged_in) {
        // Exit user out of session

        // Remove user from connected users list
        printf("Logging out %s\n", buf);
        void delete_from_list(connected_users, user);

        printf("Successfully logged out %s\n", buf);
    } else {
        printf("Successfully exited unknown user\n");
    }

    // Exit thread
    pthread_exit((void*) 0);
}

int main(int argc, char **argv) {

    if (argc != 2) {
        // Incorrect usage
        printf("Usage: socket <port_number>\n");
        exit(0);
    }

    printf("Starting server at port %s...\n", argv[1]);

    init_database();
    printf("User database loaded\n");

    // Declare variables
    int socketfd, port, num_bytes;
    char buf[BUF_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t server_addr_len, client_addr_len;
    server_addr_len = sizeof server_addr;
    client_addr_len = sizeof client_addr;

    port = htons(atoi(argv[1]));    // Get port number in network format

    // Get socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
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

    // Listen for incoming connections
    printf("Waiting for incoming connections...\n");
    if (listen(socketfd, BACKLOG)) {
        printf("Error listening to connections\n");
        exit(1);
    }

    while (true) {
        int new_sockfd = accept(socketfd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (new_sockfd < 0) {
            printf("Error accepting new connection\n");
            break;
        }

        char name[BUF_SIZE];
        inet_ntop(client_addr.ss_family, (struct sockaddr *) &client_addr, name, sizeof(name));
        printf("Received connection from %s\n", name);

        User *user = create_new_user(NULL, NULL);
        append_to_list(connected_users, user);

        pthread_create(&user->thread, NULL, (void * (*)(void *)) client_subroutine, user);
    }

    printf("Preparing for server exit\n");

    // Clear global data structures
    clear_database();

    // Close the socket
    if (close(socketfd) < 0) {
        printf("Error closing socket\n");
        exit(1);
    }

    printf("Server exited successfully\n");
    
    return 0;
}
