#ifndef TCL_USER_H
#define TCL_USER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "defines.h"

typedef struct session Session; // Forward declaration

// Holds information about a user
typedef struct user {
    char username[MAX_NAME];
    char password[MAX_NAME];

    int sockfd;

    bool logged_in;
    Session *session;

    pthread_t thread;

    // Support doubly linked list structure
    struct user *next;
    struct user *prev;
} User;

User *create_new_user(char *uname, char *pwd);
void init_user(User *user);
void free_user(User *user);

bool is_logged_in(User *user);
bool is_in_session(User *user);
bool is_valid(User *user, char *pwd);

#endif