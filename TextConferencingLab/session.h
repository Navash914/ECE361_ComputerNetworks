#ifndef TCL_SESSION_H
#define TCL_SESSION_H

#include <stdlib.h>
#include <stdbool.h>

#include "defines.h"
#include "user.h"
#include "database.h"

typedef struct session {
    char name[MAX_NAME];
    UserList *members;

    // Support linked list structure
    struct session *next;
    struct session *prev;
} Session;

Session *create_new_session(char *name);
void free_session(Session *session);

UserList *get_session_members(Session *session);
bool member_exists_in_session(Session *session, User *user);
bool add_member_to_session(Session *session, User *user);
bool remove_member_from_session(Session *session, User *user);

#endif