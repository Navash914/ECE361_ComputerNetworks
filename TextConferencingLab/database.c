#include "database.h"
#include "defines.h"

UserList *users_db = NULL;
UserList *connected_users = NULL;

void init_database() {
    users_db = (UserList *) malloc(sizeof(UserList));
    connected_users = (UserList *) malloc(sizeof(UserList));
    if (!users_db || !connected_users) {
        printf("Unable to allocate memory for user lists\n");
        exit(1);
    }

    users_db->head = NULL;
    users_db->tail = NULL;
    users_db->size = 0;

    connected_users->head = NULL;
    connected_users->tail = NULL;
    connected_users->size = 0;

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

void append_to_list(UserList *list, User *node) {
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

void delete_from_list(UserList *list, User *target) {
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
    free(target);
}

User *find_in_list(UserList *list, char *target) {
    if (list == NULL) {
        printf("No list to search in\n");
        return;
    }
    User *current = list->head;
    while (current != NULL) {
        if (!strcmp(current->username, target))
            return current;
        current = current->next;
    }
    return NULL;
}

void clear_list(UserList *list) {
    if (list == NULL) {
        printf("No list to clear\n");
        return;
    }
	while (list->head != NULL) {
		User *next = list->head->next;
        next->prev = NULL;
        list->head->next = NULL;
		free(list->head);
		list->head = next;
	}
	list->tail = NULL;
	list->size = 0;
}

void clear_database() {
    clear_list(users_db);
    clear_list(connected_users);
}