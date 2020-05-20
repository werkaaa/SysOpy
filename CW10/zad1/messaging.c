//
// Created by werka on 5/20/20.
//
#include "header.h"

void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}

client * create_client_entry(int fd, char * name){
    client * new_client = (client *) calloc(1, sizeof(client));
    new_client->fd = fd;
    strcpy(new_client->name, name);
    new_client->active = 1;
    return new_client;
}

void send_message(int sock_fd, MSG_TYPE type, char * message_data) {
    message * new_message = (message *) calloc(1, sizeof(message));
    new_message->type = type;
    strcpy(new_message->message_data, message_data);
    if(write(sock_fd, (void *)new_message, MAX_MSG_LEN) == -1) error("Problem with write function!");
    free(new_message);
}

message * read_message(int sock_fd, int block) {
    message * new_message = (message *) calloc(1, sizeof(message));

    if(block==1) {
        if (read(sock_fd, (void *) new_message, MAX_MSG_LEN) == -1) error("Problem with read function!");
    }
    else {
        if (recv(sock_fd, (void *) new_message, MAX_MSG_LEN, MSG_DONTWAIT) == -1){
            free(new_message);
            return NULL;
        }
    }

    return new_message;
}
