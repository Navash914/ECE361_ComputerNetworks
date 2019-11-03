#ifndef TCL_DATABASE_H
#define TCL_DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "user.h"

#define USER_LIST_FILE "user_list.txt"

// Defines a list of users
typedef struct userlist {
    unsigned int size;
    User *head;
    User *tail;
} UserList;

extern UserList *users_db;
extern UserList *connected_users;

void init_database();
void append_to_list(UserList *list, User *node);
void delete_from_list(UserList *list, User *target);
User *find_in_list(UserList *list, char *uname);
void clear_list(UserList *list);
void clear_database();

#endif