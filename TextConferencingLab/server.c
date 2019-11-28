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
#include <errno.h>

#include "message.h"
#include "database.h"
#include "user.h"
#include "session.h"
#include "server_actions.h"

#define LOOPBACK_ADDR "127.0.0.1"
#define BACKLOG 10
#define TIMEOUT 60

// Subroutine for each client
void client_subroutine(User *user) {
    int num_bytes;
    Message msg;
    char buf[BUF_SIZE];
    bool exiting = false;

    while (true) {
        num_bytes = recv(user->sockfd, buf, BUF_SIZE-1, FLAGS);
        if (num_bytes < 0) {
	    if (errno == EAGAIN) {
                // Timeout occurred
                printf("Logging out %s for timeout\n", user->username);
		exiting = true;
            } else {
                printf("Error receiving message from client\n");
                exit(1);
	    }
        }
        if (num_bytes == 0) {
            if (user->logged_in)
                printf("%s closed connection\n", user->username);
            else
                printf("Unknown user closed connection\n");
            exiting = true;
        }
        buf[num_bytes] = '\0';

        if (exiting)
            break;

        msg = str_to_msg(buf);
        switch (msg.type) {
            case LOGIN:
                msg = server_login(user, msg);
                break;
            case EXIT:
                exiting = true;
                break;
            case QUERY:
                msg = server_query(user, msg);
                break;
            case NEW_SESS:
                msg = server_create_session(user, msg);
                break;
            case JOIN:
                msg = server_join_session(user, msg);
                break;
            case MESSAGE:
                msg = server_message(user, msg);
                break;
            case MESSAGE_SPEC:
                msg = server_message_specific(user, msg);
                break;
            case MESSAGE_ALL:
                msg = server_message_all(user, msg);
                break;
            case INVITE:
                msg = server_invite(user, msg);
                break;
            case INVITE_LIST:
                msg = server_invite_list(user, msg);
                break;
            case INVITE_RESP:
                msg = server_invite_response(user, msg);
                break;
            case LEAVE_SESS:
                msg = server_leave_session(user, msg);
                break;
            default:
                printf("Invalid message type\n");
                break;
        }

        if (exiting)
            break;

        msg_to_str(buf, msg);
        num_bytes = send(user->sockfd, buf, BUF_SIZE-1, FLAGS);
        if (num_bytes < 0) {
            printf("Error sending message to client\n");
        }
        if (!user->logged_in)
            break;
    }

    // Client exit
    if (user->logged_in) {
        // Send reply for successful exit
	Message msg;
        msg.type = EXIT;
	msg.data[0] = '\0';
        msg.size = 0;
        strcpy(msg.source, user->username);
        msg_to_str(buf, msg);
	printf("%s\n", buf);
        num_bytes = send(user->sockfd, buf, BUF_SIZE-1, FLAGS);
        if (num_bytes < 0) {
            printf("Error sending message to client\n");
        }

        strcpy(buf, user->username);

        // Exit user out of session
        while (user->session != NULL) {
            Session *session = user->session;
            remove_member_from_session(session, user);
            printf("Removed user '%s' from session '%s'\n", user->username, session->name);
            if (session->members->size > 0) {
                Message msg;
                strcpy(msg.source, user->username);
                send_session_leave_notification(session, msg);
            } else {
                printf("Deleted session '%s' as session has no members\n", session->name);
                delete_session(sessions, session);
            }
        }

        // Remove user from connected users list
        printf("Logging out %s\n", buf);
        delete_user(connected_users, user);

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

    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    while (true) {
        int new_sockfd = accept(socketfd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (new_sockfd < 0) {
            printf("Error accepting new connection\n");
            break;
        }

        char name[BUF_SIZE];
        inet_ntop(client_addr.sin_family, (struct sockaddr *) &client_addr, name, sizeof(name));
        printf("Received connection from %s\n", name);

	if (setsockopt(new_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
	    printf("Failed to config receive timeout\n");
	    close(socketfd);
	    exit(1);
	}

        User *user = create_new_user(NULL, NULL);
        user->sockfd = new_sockfd;

        user->joined_sessions = (UserSessionList *) malloc (sizeof(UserSessionList));
        user->joined_sessions->head = NULL;
        user->joined_sessions->tail = NULL;
        user->joined_sessions->size = 0;

        user->invited_sessions = (UserSessionList *) malloc (sizeof(UserSessionList));
        user->invited_sessions->head = NULL;
        user->invited_sessions->tail = NULL;
        user->invited_sessions->size = 0;

        add_user(connected_users, user);

        pthread_create(&user->thread, NULL, (void * (*)(void *)) client_subroutine, user);
    }

    printf("Preparing for server exit...\n");

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
