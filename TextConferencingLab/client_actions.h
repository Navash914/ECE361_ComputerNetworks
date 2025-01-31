/*
* client_actions.h
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

#ifndef TCL_CLIENTACTIONS_H
#define TCL_CLIENTACTIONS_H

#include "message.h"
#include "user.h"
#include "database.h"

void print_intro();
void print_commands();

int parse_client_command(char *command);

void client_help(char *input);
int client_login(char *input, Message *msg);
bool client_logout(char *input, Message *msg);
bool client_query(char *input, Message *msg);
bool client_create_session(char *input, Message *msg);
bool client_join_session(char *input, Message *msg);
bool client_message(char *input, Message *msg);
bool client_message_specific(char *input, Message *msg);
bool client_message_all(char *input, Message *msg);
bool client_invite(char *input, Message *msg);
bool client_invite_list(char *input, Message *msg);
bool client_invite_response(char *input, Message *msg);
bool client_leave_session(char *input, Message *msg);


void client_response(Message msg);

void extract_name_and_info(char *src, char *name, char *data);

#endif
