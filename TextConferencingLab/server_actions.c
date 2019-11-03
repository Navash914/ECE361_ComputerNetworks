#include "server_actions.h"

Message server_login(User *user, Message msg) {
    User *db_entry = find_in_list(users_db, msg.source);
    if (!db_entry) {
        // User does not exist
        msg.type = LO_NAK;
        strcpy(msg.data, "User does not exist\0");
        msg.size = strlen(msg.data);
        return msg;
    }

    if (!is_valid(db_entry, msg.data)) {
        // Incorrect password
        msg.type = LO_NAK;
        strcpy(msg.data, "Incorrect password\0");
        msg.size = strlen(msg.data);
        return msg;
    }

    msg.type = LO_ACK;
    msg.size = 0;
    return msg;
}

Message server_query(User *user, Message msg) {
    char buf[MAX_DATA];
    if (connected_users->size == 0)
        strcpy(buf, "There are no users online\n");
    else {
        strcpy(buf, "Online users:");
        User *current = connected_users->head;
        while (current != NULL) {
            strcat(buf, " ");
            strcat(buf, current->username);
            current = current->next;
        }
        strcat(buf, "\n");
    }

    // TODO: Concat session info in buf

    msg.type = QU_ACK;
    strcpy(msg.data, buf);
    msg.size = strlen(msg.data);
    return msg;
}

Message server_create_session(User *user, Message msg) {
    char buf[MAX_DATA];
    // TODO: Check if session exists. Return NAK if it does

    // TODO: Create and join session and return session name in msg
}

Message server_join_session(User *user, Message msg) {
    char buf[MAX_DATA];
    // TODO: Check if session exists. Return NAK if it doesn't

    // TODO: Join session and return session name in msg
}

Message server_message(User *user, Message msg) {
    char buf[MAX_DATA];
    // TODO: Check if user is in session. If not, send NAK

    // TODO: Send message to all users in session
}

Message server_leave_session(User *user, Message msg) {
    char buf[MAX_DATA];
    // TODO: Check if session exists. Return NAK if it doesn't

    // TODO: Leave session and return session name in msg
}