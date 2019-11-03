#include "server_actions.h"

Message client_login(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message client_query(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message client_create_session(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message client_join_session(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message client_message(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message client_leave_session(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}