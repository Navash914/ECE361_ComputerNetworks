#ifndef TCL_SERVERACTIONS_H
#define TCL_SERVERACTIONS_H

#include "message.h"
#include "user.h"
#include "database.h"

Message server_login(User *user, Message msg);
Message server_query(User *user, Message msg);
Message server_create_session(User *user, Message msg);
Message server_join_session(User *user, Message msg);
Message server_message(User *user, Message msg);
Message server_leave_session(User *user, Message msg);

#endif