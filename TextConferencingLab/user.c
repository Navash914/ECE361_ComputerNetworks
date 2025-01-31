/*
* user.c
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

#include "user.h"

User *create_new_user(char *uname, char *pwd) {
    User *user = (User *) malloc(sizeof(User));
    if (!user) {
        printf("Unable to allocate memory for new user\n");
        exit(1);
    }
    if (uname != NULL)
        strcpy(user->username, uname);
    if (pwd != NULL)
        strcpy(user->password, pwd);

    init_user(user);
    return user;
}

void init_user(User *user) {
    user->sockfd = -1;
    user->logged_in = false;
    user->session = NULL;
    user->joined_sessions = NULL;

    user->next = NULL;
    user->prev = NULL;
}

void free_user(User *user) {
    free(user);
}

User *copy_user(User *user) {
    User *new_user = (User *) malloc(sizeof(User));
    memcpy(new_user, user, sizeof(User));
    new_user->next = NULL;
    new_user->prev = NULL;
    return new_user;
}

bool is_logged_in(User *user) {
    return user->logged_in;
}

bool is_in_session(User *user) {
    return user->session != NULL;
}

bool is_valid(User *user, char *pwd) {
    return !strcmp(user->password, pwd);
}
