/*
* message.h
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

#ifndef TCL_MESSAGE_H
#define TCL_MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "defines.h"

#define EXIT 0

#define LOGIN 1
#define LO_ACK 2
#define LO_NAK 3

#define QUERY 4
#define QU_ACK 5
#define QU_NAK 6

#define NEW_SESS 7
#define NS_ACK 8
#define NS_NAK 9

#define JOIN 10
#define JN_ACK 11
#define JN_NAK 12

#define MESSAGE 13
#define MS_ACK 14
#define MS_NAK 15

#define LEAVE_SESS 16
#define LV_ACK 17
#define LV_NAK 18

#define NOTIFICATION 19

#define HELP 20

typedef struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
} Message;

size_t msg_to_str(char *dest, Message msg);
Message str_to_msg(char *src);

Message copy_message(Message src);

#endif
