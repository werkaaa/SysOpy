//
// Created by werka on 4/20/20.
//

#ifndef SYSOPY_HEADER_H
#define SYSOPY_HEADER_H
#define MAX_CLIENTS 4
#define NAME_SIZE 5
#define MAX_MSG_SIZE 128

const char * SERVER_FILE_NAME = "/server";

typedef enum m_type {
    STOP = 6, DISCONNECT = 5, LIST = 4, INIT = 3, CONNECT = 2, CHAT = 1
} m_type;

typedef struct message_buf {
    long m_type;
    char m_text[MAX_MSG_SIZE];
    key_t queue_key;
    int client_id;
    int partner_id;
} message_buf;


#endif //SYSOPY_HEADER_H
