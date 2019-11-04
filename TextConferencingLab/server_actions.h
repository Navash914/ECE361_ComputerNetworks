#ifndef TCL_SERVERACTIONS_H
#define TCL_SERVERACTIONS_H

#include <sys/types.h>
#include <sys/socket.h>

#include "message.h"
#include "user.h"
#include "database.h"

Message server_login(User *user, Message msg);
Message server_query(User *user, Message msg);
Message server_create_session(User *user, Message msg);
Message server_join_session(User *user, Message msg);
Message server_message(User *user, Message msg);
Message server_leave_session(User *user, Message msg);

void send_session_creation_notification(Session *session, Message msg);
void send_session_join_notification(Session *session, Message msg);
void send_session_leave_notification(Session *session, Message msg);

void server_broadcast(UserList *list, Message msg);

#endif