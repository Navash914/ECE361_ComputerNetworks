/*
* message.c
*
* Written By:
* Naveed Ashfaq - #1003859559
* Yu Dong Feng - #1002587523
*/

#include "message.h"

// Converts a message struct into a string
// Format => type size source data
// Returns length of string
size_t msg_to_str(char *dest, Message msg) {
    sprintf(dest, "%u %u %s ", 
            msg.type, msg.size, 
            msg.source);
    size_t len = strlen(dest) + msg.size;
    char *s = dest;
    s += strlen(dest);
    for (int i=0; i<msg.size; ++i) {
        *s++ = msg.data[i];
    }
    *s = '\0';
    return len;
}

// Builds a struct message from its string representation
// Format => type size source data
// Returns the struct message
Message str_to_msg(char *src) {
    Message msg;
    sscanf(src, "%u %u %s", 
            &msg.type, &msg.size, 
            msg.source);
    
    // Position in src that filedata starts from
    size_t pos = floor(log10(msg.type))  // Length of the digits in type
                + floor(log10(msg.size))    // Length of the digits in size
                + strlen(msg.source)         // Length of source
                + 5;    // +2 from the 2 integer values as floor(log(x)) gives 1 less than the actual no. of digits in x
                        // +3 from the 3 spaces in the format of the string

    char *s = src;
    s += pos;
    for (int i=0; i<msg.size; ++i) {
        msg.data[i] = *s++;
    }

    msg.data[msg.size] = '\0';

    return msg;
}

Message copy_message(Message src) {
    Message msg;
    msg.type = src.type;
    msg.size = src.size;
    strcpy(msg.source, src.source);
    strcpy(msg.data, src.data);
    return msg;
}
