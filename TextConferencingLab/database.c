#include "database.h"
#include "defines.h"

UserList *users_db = NULL;
UserList *connected_users = NULL;
SessionList *sessions;

void init_database() {
    users_db = (UserList *) malloc(sizeof(UserList));
    connected_users = (UserList *) malloc(sizeof(UserList));
    sessions = (SessionList *) malloc(sizeof(SessionList));
    if (!users_db || !connected_users || !sessions) {
        printf("Unable to allocate memory for lists\n");
        exit(1);
    }

    users_db->head = NULL;
    users_db->tail = NULL;
    users_db->size = 0;

    connected_users->head = NULL;
    connected_users->tail = NULL;
    connected_users->size = 0;

    sessions->head = NULL;
    sessions->tail = NULL;
    sessions->size = 0;

    // Check if file exists
    if(access(USER_LIST_FILE, F_OK) == -1){
		printf("Unable to find ");
        printf(USER_LIST_FILE);
        printf("\n");
		exit(1);
	}

    FILE *userlist_file;
    file = fopen(USER_LIST_FILE, BINARY_READ_MODE);
    if (!file) {
        printf("Error opening file\n");
        exit(1);
    }

    char uname[MAX_NAME], pwd[MAX_NAME];

    while (fscanf(userlist_file, "%s %s\n", uname, pwd) != EOF) {
        User *user = create_new_user(uname, pwd);
        append_to_list(user);
    }
}

void clear_database() {
    clear_user_list(users_db);
    clear_user_list(connected_users);
    clear_session_list(sessions);
    free(users_db);
    free(connected_users);
    free(sessions);
}

void add_user(UserList *list, User *node) {
    if (list == NULL) {
        printf("No list to append to\n");
        return;
    }
    if (list->head == NULL) {
		// List is currently empty
		list->head = node;
		list->tail = node;
	} else {
		list->tail->next = node;	// Append to end of list
        node->prev = list->tail;
		list->tail = list->tail->next;	// Move tail to new end of list
	}
	list->size++;	// Update list size
}

void delete_user(UserList *list, User *target) {
    if (list == NULL) {
        printf("No list to delete from\n");
        return;
    }
    if (list->size == 1) {
		// This is the only node in the list
		list->head = NULL;
		list->tail = NULL;
	} else if (target == list->head) {
		// Removing head
		list->head = list->head->next;
        list->head->prev = NULL;
		target->next = NULL;
	} else {
        User *prev = target->prev;
		prev->next = target->next;
		target->next = NULL;
        target->prev = NULL;
		if (target == list->tail) {
			// Update new tail
			list->tail = prev;
		} else {
            prev->next->prev = prev;
        }
	}
	list->size--;	// Update list size
    free_user(target);
}

User *find_user(UserList *list, char *uname) {
    if (list == NULL) {
        printf("No list to search in\n");
        return;
    }
    User *current = list->head;
    while (current != NULL) {
        if (!strcmp(current->username, uname))
            return current;
        current = current->next;
    }
    return NULL;
}

void clear_user_list(UserList *list) {
    if (list == NULL) {
        printf("No list to clear\n");
        return;
    }
	while (list->head != NULL) {
		User *next = list->head->next;
        next->prev = NULL;
        list->head->next = NULL;
		free_user(list->head);
		list->head = next;
	}
	list->tail = NULL;
	list->size = 0;
}

void add_session(SessionList *list, Session *node) {
    if (list == NULL) {
        printf("No list to append to\n");
        return;
    }
    if (list->head == NULL) {
		// List is currently empty
		list->head = node;
		list->tail = node;
	} else {
		list->tail->next = node;	// Append to end of list
        node->prev = list->tail;
		list->tail = list->tail->next;	// Move tail to new end of list
	}
	list->size++;	// Update list size
}

void delete_session(SessionList *list, Session *target) {
    if (list == NULL) {
        printf("No list to delete from\n");
        return;
    }
    if (list->size == 1) {
		// This is the only node in the list
		list->head = NULL;
		list->tail = NULL;
	} else if (target == list->head) {
		// Removing head
		list->head = list->head->next;
        list->head->prev = NULL;
		target->next = NULL;
	} else {
        User *prev = target->prev;
		prev->next = target->next;
		target->next = NULL;
        target->prev = NULL;
		if (target == list->tail) {
			// Update new tail
			list->tail = prev;
		} else {
            prev->next->prev = prev;
        }
	}
	list->size--;	// Update list size

    free_session(target);
}

Session *find_session(SessionList *list, char *name) {
    if (list == NULL) {
        printf("No list to search in\n");
        return;
    }
    User *current = list->head;
    while (current != NULL) {
        if (!strcmp(current->name, name))
            return current;
        current = current->next;
    }
    return NULL;
}

void clear_session_list(SessionList *list) {
    if (list == NULL) {
        printf("No list to clear\n");
        return;
    }
	while (list->head != NULL) {
		User *next = list->head->next;
        next->prev = NULL;
        list->head->next = NULL;
		free_session(list->head);
		list->head = next;
	}
	list->tail = NULL;
	list->size = 0;
}