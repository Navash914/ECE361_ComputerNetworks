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
} Session;



#endif