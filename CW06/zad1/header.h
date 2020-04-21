//
// Created by werka on 4/20/20.
//

#ifndef SYSOPY_HEADER_H
#define SYSOPY_HEADER_H
#define MAX_CLIENTS 4
#define MAX_MSG_SIZE 128

const int SERVER_GEN = 1;

typedef enum m_type {
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5, CHAT = 6
} m_type;

typedef struct message_buf {
    long m_type;
    char m_text[MAX_MSG_SIZE];
    key_t queue_key;
    int client_id;
    int partner_id;
} message_buf;


#endif //SYSOPY_HEADER_H
