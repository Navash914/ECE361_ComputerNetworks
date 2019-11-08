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

UserList *get_session_members(Session *session) {
    return session->members;
}

bool member_exists_in_session(Session *session, User *user) {
    return user->session == session;
}

bool add_member_to_session(Session *session, User *user) {
    if (user->session != NULL)
        return false;
    User *user_in_session = copy_user(user);
    add_user(session->members, user_in_session);
    user->session = session;
    return true;
}

bool remove_member_from_session(Session *session, User *user) {
    User *user_in_session = find_user(session->members, user->username);
    if (user_in_session == NULL)
        return false;
    delete_user(session->members, user_in_session);
    user->session = NULL;
    return true;
}