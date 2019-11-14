/*
* database.h
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

#ifndef TCL_DATABASE_H
#define TCL_DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "user.h"
#include "session.h"

#define USER_LIST_FILE "user_list.txt"

// Defines a list of users
typedef struct userlist {
    unsigned int size;
    User *head;
    User *tail;
} UserList;

// Defines a list of sessions
typedef struct sessionlist {
    unsigned int size;
    Session *head;
    Session *tail;
} SessionList;

extern UserList *users_db;
extern UserList *connected_users;
extern SessionList *sessions;

void init_database();
void clear_database();

// UserList operations
void add_user(UserList *list, User *node);
void delete_user(UserList *list, User *target);
User *find_user(UserList *list, char *uname);
void clear_user_list(UserList *list);

// SessionList operations
void add_session(SessionList *list, Session *node);
void delete_session(SessionList *list, Session *target);
Session *find_session(SessionList *list, char *name);
void clear_session_list(SessionList *list);

#endif
