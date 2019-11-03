#ifndef TCL_SERVERSERVICES_H
#define TCL_SERVERSERVICES_H

#include "message.h"
#include "user.h"
#include "database.h"

Message client_login(User *user, Message msg);
Message client_query(User *user, Message msg);
Message client_create_session(User *user, Message msg);
Message client_join_session(User *user, Message msg);
Message client_message(User *user, Message msg);
Message client_leave_session(User *user, Message msg);

#endif