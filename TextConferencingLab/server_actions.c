#include "server_actions.h"

Message server_login(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);

}

Message server_query(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message server_create_session(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message server_join_session(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message server_message(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}

Message server_leave_session(User *user, Message msg) {
    char buf[MAX_DATA];
    memcpy(buf, msg.data, msg.size);
    
}