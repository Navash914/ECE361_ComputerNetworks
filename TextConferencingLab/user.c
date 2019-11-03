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
    user->session_id = -1;

    user->next = NULL;
    user->prev = NULL;
}

bool is_logged_in(User *user) {
    return user->logged_in;
}

bool is_in_session(User *user) {
    return user->session_id > 0;
}

bool is_valid(User *user, char *pwd) {
    return !strcmp(user->password, pwd);
}