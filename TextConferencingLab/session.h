/*
* session.h
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

#ifndef TCL_SESSION_H
#define TCL_SESSION_H

#include <stdlib.h>
#include <stdbool.h>

#include "defines.h"
#include "user.h"
#include "database.h"

typedef struct userlist UserList;

typedef struct session {
    char name[MAX_NAME];
    UserList *members;

    // Support linked list structure
    struct session *next;
    struct session *prev;
} Session;

typedef struct usersession {
    Session *session;

    // Support linked list structure
    struct usersession *next;
    struct usersession *prev;
} UserSession;

Session *create_new_session(char *name);
void free_session(Session *session);

UserSession* create_usersession_node(Session *session);

UserList *get_session_members(Session *session);
bool member_exists_in_session(Session *session, User *user);

bool add_member_to_session(Session *session, User *user);
bool remove_member_from_session(Session *session, User *user);

bool invite_user_to_session(Session *session, User *user);
bool accept_user_invite_to_session(Session *session, User *user);
bool decline_user_invite_to_session(Session *session, User *user);
bool user_invited_to_session(Session *session, User *user);

#endif
