/*
* client_actions.c
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include "client_actions.h"

void print_intro() {
    printf("\n=== Welcome to the Multi-Party Text Conferencing App! ===\n\n");
    printf("To get started, login to a server with your username and password.\n");
    printf("\nAvailable commands:\n  /login <username> <password> <server-ip> <server-port>\n");
    printf("\nYou can use /quit to exit the program anytime\n\n");
}

void print_commands() {
    printf("\nAvailable commands to communicate with the server:\n");
    printf("  /logout\n");
    printf("  /createsession <session_name>\n");
    printf("  /joinsession <session_name>\n");
    printf("  /leavesession [session_name]\n");
    printf("  /list\n");
    printf("  /all <message>\n");
    printf("  /msg <session_name> <message>\n");
    printf("  /help\n");    // TODO: Add functionality for expanded help
    printf("\nWhile in a session, simply type your message to send it to other users in the session.\n");
    printf("\nYou can use /quit to exit the program anytime\n\n");
}

int parse_client_command(char *command) {
    if (command[0] != '/')
        return MESSAGE;
    if (!strcmp(command, "/login"))
        return LOGIN;
    else if (!strcmp(command, "/logout"))
        return EXIT;
    else if (!strcmp(command, "/joinsession"))
        return JOIN;
    else if (!strcmp(command, "/leavesession"))
        return LEAVE_SESS;
    else if (!strcmp(command, "/createsession"))
        return NEW_SESS;
    else if (!strcmp(command, "/list"))
        return QUERY;
    else if (!strcmp(command, "/help"))
        return HELP;
    else if (!strcmp(command, "/quit"))
        return -1;
    else if (!strcmp(command, "/msg"))
        return MESSAGE_SPEC;
    else if (!strcmp(command, "/all"))
        return MESSAGE_ALL;
    else
        return MESSAGE;
}

int client_login(char *input, Message *msg) {
    // Input format: /login <username> <password> <server-ip> <server-port>
    char username[MAX_NAME], password[MAX_NAME], server_ip[MAX_NAME];
    int port_number;
    int r = sscanf(input, "/login %s %s %s %d", username, password, server_ip, &port_number);
    if (r < 4) {
        printf("Invalid usage of /login command.\nUsage: /login <username> <password> <server-ip> <server-port>\n");
        return -1;
    }

    // Declare Variables
    int socketfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len;
    server_addr_len = sizeof server_addr;

    port_number = htons(port_number);    // Get port number in network format

    // Get socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        printf("Error acquiring socket\n");
        exit(1);
    }

    // Create the address struct for server
    memset(&server_addr, 0, server_addr_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = port_number;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    inet_aton(server_ip, &server_addr.sin_addr);

    if (connect(socketfd, (struct sockaddr *) &server_addr, server_addr_len)) {
        printf("Error connecting to server");
        return -1;
    }

    strcpy(msg->source, username);
    strcpy(msg->data, password);
    msg->size = strlen(password);
    return socketfd;

}

bool client_logout(char *input, Message *msg) {
    // Input format: /logout

    //if (close(socketfd)) {
    //    printf("Error closing socket\n");
    //    exit(1);
    //}

    msg->size = 0;
    return true;
}

bool client_query(char *input, Message *msg) {
    // Input format: /list
    // Ignoring any text after the /list, this cannot have a usage error
    msg->size = 0;
    return true;
}

bool client_create_session(char *input, Message *msg) {
    // Input format: /createsession <sessionID>
    char session_name[MAX_NAME];
    int r = sscanf(input, "/createsession %s", session_name);
    if (r < 1) {
        printf("Invalid usage of /createsession command.\nUsage: /createsession <sessionID>\n");
        return false;
    }

    strcpy(msg->data, session_name);
    msg->size = strlen(session_name);
    return true;
}

bool client_join_session(char *input, Message *msg) {
    // Input format: /joinsession <sessionID>
    char session_name[MAX_NAME];
    int r = sscanf(input, "/joinsession %s", session_name);
    if (r < 1) {
        printf("Invalid usage of /joinsession command.\nUsage: /joinsession <sessionID>\n");
        return false;
    }

    strcpy(msg->data, session_name);
    msg->size = strlen(session_name);
    return true;
}

bool client_message(char *input, Message *msg) {
    // Input format: none
    int input_size = strlen(input);
    if (input_size > MAX_DATA) {
        printf("Message too large to send\n");
        return false;
    }
    strcpy(msg->data, input);
    msg->size = input_size;
    return true;
}

bool client_message_specific(char *input, Message *msg) {
    // Input format: /msg <session_name> <message>
    char buf[MAX_NAME];
    char message[MAX_DATA];
    int r = sscanf(input, "/msg %s", buf);
    if (r < 1) {
        printf("Invalid usage of /msg command.\nUsage: /msg <session_name> <message>\n");
        return false;
    }
    input += strlen("/msg ");
    extract_name_and_info(input, buf, message);
    int msg_size = strlen(message);
    if (msg_size == 0) {
        printf("Invalid usage of /msg command.\nUsage: /msg <session_name> <message>\n");
        return false;
    } else if (msg_size + strlen(buf) + 1 > MAX_DATA) {
        printf("Message too large to send\n");
        return false;
    }
    sprintf(msg->data, "%s %s", buf, message);
    msg->size = strlen(msg->data);
    return true; 
}

bool client_message_all(char *input, Message *msg) {
    // Input format: /all <message>
    char buf[MAX_NAME];
    char message[MAX_DATA];
    extract_name_and_info(input, buf, message);
    int msg_size = strlen(message);
    if (msg_size == 0) {
        printf("Invalid usage of /all command.\nUsage: /all <message>\n");
        return false;
    } else if (msg_size > MAX_DATA) {
        printf("Message too large to send\n");
        return false;
    }
    strcpy(msg->data, message);
    msg->size = msg_size;
    return true; 
}

bool client_leave_session(char *input, Message *msg) {
    // Input format: /leavesession <optional:session_name>
    char session_name[MAX_NAME];
    int r = sscanf(input, "/leavesession %s", session_name);
    if (r < 1) {
        msg->size = 0;
    } else {
        strcpy(msg->data, session_name);
        msg->size = strlen(session_name);
    }
        
    return true;
}

void client_response(Message msg) {
    char name_buf[MAX_NAME], data_buf[MAX_DATA];
    switch(msg.type) {
        case LO_ACK:
            printf("%s successfully logged in.\n", msg.source);
            print_commands();
            break;
        case LO_NAK:
            printf("Could not log in %s.\nReason: %s\n", msg.source, msg.data);
            break;
        case QU_ACK:
            printf("\n%s", msg.data);
            break;
        case QU_NAK:
            printf("Could not successfully obtain query.\nReason: %s\n", msg.data);
            break;
        case NS_ACK:
            printf("Successfully created and joined session '%s'\n", msg.data);
            break;
        case NS_NAK:
            extract_name_and_info(msg.data, name_buf, data_buf);
            printf("Could not create session '%s'.\nReason: %s\n", name_buf, data_buf);
            break;
        case JN_ACK:
            {
            int is_new = 0;
            sscanf(msg.data, "%d %s", &is_new, name_buf);
            if (is_new)        
                printf("Successfully joined and moved active session to '%s'\n", name_buf);
            else
                printf("Successfully moved active session to '%s'\n", name_buf);
            break;
            }
        case JN_NAK:
            extract_name_and_info(msg.data, name_buf, data_buf);
            printf("Could not join session '%s'.\nReason: %s\n", name_buf, data_buf);
            break;
        case NOTIFICATION:
            printf("\nSERVER NOTIFICATION => %s\n\n", msg.data);
            break;
        case MESSAGE:
            extract_name_and_info(msg.data, name_buf, data_buf);
            printf("[%s] %s: %s", name_buf, msg.source, data_buf);
            //printf("%s: %s", msg.source, msg.data);
            break;
        case MS_ACK:
            break;
        case MS_NAK:
            printf("Could not send message.\nReason: %s\n",msg.data);
            break;
        case LV_ACK:
            {
            int r = sscanf(msg.data, "%s %s", name_buf, data_buf);
            printf("Successfully left session '%s'\n", name_buf);
            if (r > 1)
                printf("New active session is '%s'\n", data_buf);
            break;
            }
        case LV_NAK:
            printf("Could not leave session.\nReason: %s\n", msg.data);
            break;    
    }
}

void extract_name_and_info(char *src, char *name, char *data) {
    char *s = src, *d = name;
    while (*s != ' ' && *s != '\0')
        *d++ = *s++;
    *d = '\0';

    d = data;
    if (*s != '\0')
        s++;
    while (*s != '\0')
        *d++ = *s++;
    *d = '\0';
}
