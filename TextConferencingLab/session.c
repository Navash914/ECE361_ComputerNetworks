/*
* session.c
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

#include "session.h"

Session *create_new_session(char *name) {
    Session *session = (Session *) malloc(sizeof(Session));
    if (!session) {
        printf("Could not allocate memory for session");
        exit(1);
    }

    session->members = (UserList *) malloc(sizeof(UserList));
    if (!session->members) {
        printf("Could not allocate memory for session");
        free(session);
        exit(1);
    }

    strcpy(session->name, name);
    session->members->head = NULL;
    session->members->tail = NULL;
    session->members->size = 0;

    session->next = NULL;
    session->prev = NULL;

    return session;
}

void free_session(Session *session) {
    clear_user_list(session->members);
    free(session->members);
    free(session);
}

UserSession* create_usersession_node(Session *session) {
    UserSession *node = (UserSession *) malloc (sizeof(UserSession));
    node->session = session;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

UserList *get_session_members(Session *session) {
    return session->members;
}

bool member_exists_in_session(Session *session, User *user) {
    if (user->session == session)
        return true;
    if (find_user(session->members, user->username))
        return true;
    return false;
}

bool add_member_to_session(Session *session, User *user) {
    if (member_exists_in_session(session, user)) {
        user->session = session;
        return false;
    }
    User *user_in_session = copy_user(user);
    add_user(session->members, user_in_session);
    add_usersession(user->joined_sessions, create_usersession_node(session));
    user->session = session;
    return true;
}

bool remove_member_from_session(Session *session, User *user) {
    User *user_in_session = find_user(session->members, user->username);
    if (user_in_session == NULL)
        return false;
    delete_user(session->members, user_in_session);
    delete_usersession(user->joined_sessions, find_usersession(user->joined_sessions, session));
    if (user->session == session) {
        if (user->joined_sessions->size == 0)
            user->session = NULL;
        else
            user->session = user->joined_sessions->head->session;
    }
    return true;
}
