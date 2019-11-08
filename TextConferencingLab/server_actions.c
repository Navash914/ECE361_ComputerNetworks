#include "server_actions.h"

Message server_login(User *user, Message msg) {
    User *db_entry = find_user(users_db, msg.source);
    if (!db_entry) {
        // User does not exist
        msg.type = LO_NAK;
        strcpy(msg.data, "User does not exist\0");
        msg.size = strlen(msg.data);
        return msg;
    }

    if (!is_valid(db_entry, msg.data)) {
        // Incorrect password
        printf("Expected password: %s\nReceived password: %s\n", db_entry->password, msg.data);
        msg.type = LO_NAK;
        strcpy(msg.data, "Incorrect password\0");
        msg.size = strlen(msg.data);
        return msg;
    }

    strcpy(user->username, db_entry->username);
    strcpy(user->password, db_entry->password);
    user->logged_in = true;
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
    if (sessions->size == 0)
        strcat(buf, "There are currently no available sessions.\n");
    else {
        strcat(buf, "Currently available sessions: \n");
        Session *current = sessions->head;
        while (current != NULL) {
            strcat(buf, "  ");
            strcat(buf, current->name);
            strcat(buf, ": \n");
            User *user = current->members->head;
            while (user != NULL) {
                strcat(buf, "    -> ");
                strcat(buf, user->username);
                strcat(buf, "\n");
                user = user->next;
            }
            current = current->next;
        }
        strcat(buf, "\n");
    }

    msg.type = QU_ACK;
    strcpy(msg.data, buf);
    msg.size = strlen(msg.data);
    return msg;
}

Message server_create_session(User *user, Message msg) {
    char buf[MAX_DATA];
    // TODO: Check if session exists. Return NAK if it does
    if (find_session(sessions, msg.data) != NULL) {
        // Session exits. Cannot create session with duplicate name
        msg.type = NS_NAK;
        sprintf(buf, "%s Session already exists.\0", msg.data);
        strcpy(msg.data, buf);
        msg.size = strlen(msg.data);
        return msg;
    }

    if (user->session != NULL) {
        msg.type = NS_NAK;
        sprintf(buf, "%s You are already a member of '%s', please leave that session before creating a new one.\0", msg.data, user->session->name);
        strcpy(msg.data, buf);
        msg.size = strlen(msg.data);
        return msg;
    }

    // TODO: Create and join session and return session name in msg
    Session *session = create_new_session(msg.data);
    add_member_to_session(session, user);
    add_session(sessions, session);
    msg.type = NS_ACK;
    send_session_creation_notification(session, msg);
    return msg;
}

Message server_join_session(User *user, Message msg) {
    char buf[MAX_DATA];
    // TODO: Check if session exists. Return NAK if it doesn't
    Session *session = find_session(sessions, msg.data);
    if (session == NULL) {
        // Session does not exist
        msg.type = JN_NAK;
        sprintf(buf, "%s Session does not exist.\0", msg.data);
        strcpy(msg.data, buf);
        msg.size = strlen(msg.data);
        return msg;
    }

    // TODO: Join session and return session name in msg
    if (user->session == session) {
        msg.type = JN_NAK;
        sprintf(buf, "%s You are already a member of this session.\0", msg.data);
        strcpy(msg.data, buf);
        msg.size = strlen(msg.data);
        return msg;
    }

    if (user->session != NULL) {
        msg.type = JN_NAK;
        sprintf(buf, "%s You are already a member of '%s', please leave that session before joining a new one.\0", msg.data, user->session->name);
        strcpy(msg.data, buf);
        msg.size = strlen(msg.data);
        return msg;
    }

    add_member_to_session(session, user);
    msg.type = JN_ACK;
    send_session_join_notification(session, msg);
    return msg;
}

Message server_message(User *user, Message msg) {
    // TODO: Check if user is in session. If not, send NAK
    if (user->session == NULL) {
        msg.type = MS_NAK;
        strcpy(msg.data, "Please join a session before sending messages.\0");
        msg.size = strlen(msg.data);
        return msg;
    }

    // TODO: Send message to all users in session
    UserList *members = user->session->members;
    server_broadcast(members, msg);
    msg.type = MS_ACK;
    return msg;
}

Message server_leave_session(User *user, Message msg) {
    char buf[MAX_DATA];

    Session *session = user->session;

    if (session == NULL) {
        msg.type = LV_NAK;
        strcpy(msg.data, "You are not a member of any session.\0");
        msg.size = strlen(msg.data);
        return msg;
    }

    strcpy(msg.data, session->name);
    msg.size = strlen(msg.data);
    remove_member_from_session(session, user);
    msg.type = LV_ACK;
    if (session->members->size > 0)
        send_session_leave_notification(session, msg);
    else
        delete_session(sessions, session);
    return msg;
}

void send_session_creation_notification(Session *session, Message msg) {
    char buf[BUF_SIZE];
    sprintf(buf, "%s has created a new session: %s", msg.source, msg.data);
    strcpy(msg.data, buf);
    msg.size = strlen(msg.data);
    msg.type = NOTIFICATION;
    server_broadcast(connected_users, msg);
}

void send_session_join_notification(Session *session, Message msg) {
    char buf[BUF_SIZE];
    sprintf(buf, "%s has joined the session!", msg.source);
    strcpy(msg.data, buf);
    msg.size = strlen(msg.data);
    msg.type = NOTIFICATION;
    server_broadcast(session->members, msg);
}

void send_session_leave_notification(Session *session, Message msg) {
    char buf[BUF_SIZE];
    sprintf(buf, "%s has left the session.", msg.source);
    strcpy(msg.data, buf);
    msg.size = strlen(msg.data);
    msg.type = NOTIFICATION;
    server_broadcast(session->members, msg);
}

void server_broadcast(UserList *list, Message msg) {
    printf("Starting broadcast.\nMsg source is ");
    printf("%s\n", msg.source);
    char buf[BUF_SIZE];
    int num_bytes;
    msg_to_str(buf, msg);
    User *user = list->head;
    while (user != NULL) {
        if (!strcmp(user->username, msg.source)) {
            user = user->next;
            continue;
        }
        printf("Broadcasting to %s\n", user->username);
        num_bytes = send(user->sockfd, buf, BUF_SIZE-1, FLAGS);
        if (num_bytes < 0) {
            printf("Error sending message to %s\n", user->username);
        }
        user = user->next;
    }
}