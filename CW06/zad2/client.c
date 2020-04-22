//
// Created by werka on 4/20/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "header.h"

mqd_t client_queue_id;
mqd_t server_queue_id;
char client_queue_name[NAME_SIZE+1];
int client_id;
mqd_t partner_queue_id = -1;
int partner_id = -1;

void error(char* msg) {
    printf("Error: %s", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}

void bye(void){
    if(mq_unlink(client_queue_name)==-1){
        error("Problem with queue deletion!\n");
    }
}

void stop(int signum){
    char s_msg[MAX_MSG_SIZE];
    unsigned int s_priority;
    if(partner_id!=-1){
        s_priority = DISCONNECT;
        snprintf(s_msg, MAX_MSG_SIZE, "%d %d", partner_id, client_id);
        if(mq_send(partner_queue_id, s_msg, strlen(s_msg), s_priority) == -1){
            error("Problem with sending a closing message!\n");
        }
        if(mq_send(server_queue_id, s_msg, strlen(s_msg), s_priority) == -1){
            error("Problem with sending a closing message!\n");
        }
    }

    s_priority = STOP;
    snprintf(s_msg, MAX_MSG_SIZE, "%d", client_id);
    if(mq_send(server_queue_id, s_msg, strlen(s_msg), s_priority) == -1){
        error("Problem with sending a message!\n");
    }

    if(mq_close(server_queue_id) == -1){
        error("Problem with closing the server queue!\n");
    }

    exit(0);
}

void handle_command(char * command){

    unsigned int s_priority;
    char s_msg[MAX_MSG_SIZE];
    memset(s_msg, '\0', MAX_MSG_SIZE);

    if(strcmp(command, "LIST")==0){
        printf("Handling LIST command\n");
        s_priority = LIST;
        if(mq_send(server_queue_id, s_msg, sizeof(s_msg), s_priority) == -1){
            error("Problem with sending a message!\n");
        }
    }
    else if(strcmp(command, "CONNECT")==0){
        printf("Handling CONNECT command\n");
        int partner_id;
        scanf("%d", &partner_id);
        s_priority = CONNECT;
        snprintf(s_msg, MAX_MSG_SIZE, "%d %d ", client_id, partner_id);
        if(mq_send(server_queue_id, s_msg, sizeof(s_msg), s_priority) == -1){
            error("Problem with sending a message!\n");
        }

    }
    else if(strcmp(command, "STOP")==0){
        stop(0);
    }
    else{
        printf("Command '%s' not known!\n", command);
    }

}

int handle_message(unsigned int priority, char * msg){

    char * tok;
    struct mq_attr attr;

    switch(priority){

        case CONNECT:
            printf("Client received CONNECT\n");

            tok = strtok(msg, " ");
            tok = strtok(NULL, " ");
            partner_id = atoi(tok);
            tok = strtok(NULL, " ");

            partner_queue_id = mq_open(tok, O_WRONLY);
            if(partner_queue_id == -1)
                error("Problem with connection!\n");

            char s_msg[MAX_MSG_SIZE];
            unsigned int s_priority;

            char msg_text[MAX_MSG_SIZE];

            struct timeval tmo;
            fd_set readfds;
            printf("Enter message for client %d:", partner_id);
            fflush(stdout);
            while(1){

                FD_ZERO(&readfds);
                FD_SET(0, &readfds);
                tmo.tv_sec = 1;
                tmo.tv_usec = 0;

                switch (select(1, &readfds, NULL, NULL, &tmo)) {
                    case -1:
                        error("Error while using select!\n");
                        break;
                    case 1:
                        scanf("%s", msg_text);
                        if(strcmp(msg_text, "DISCONNECT")==0){
                            s_priority = DISCONNECT;

                            snprintf(s_msg, MAX_MSG_SIZE, "%d %d ", client_id, partner_id);

                            if(mq_send(server_queue_id, s_msg, sizeof(s_msg), s_priority) == -1){
                                error("Problem with sending a message!\n");
                            }
                            if(mq_send(partner_queue_id, s_msg, sizeof(s_msg), s_priority) == -1){
                                error("Problem with sending a message!\n");
                            }

                            if(mq_close(partner_queue_id) == -1){
                                error("Problem with closing the server queue!\n");
                            }

                            partner_queue_id = -1;
                            partner_id = -1;

                            printf("Enter command:");
                            fflush(stdout);
                            return 0;
                        }
                        s_priority = CHAT;

                        if(mq_send(partner_queue_id, msg_text, sizeof(msg_text), s_priority) == -1){
                            error("Problem with sending a chat message!\n");
                        }
                        printf("Enter message for client %d:", partner_id);
                        fflush(stdout);
                        break;
                }

                if(mq_getattr(client_queue_id, &attr) == -1)
                    error("Problem with getting queue info!\n");
                if(attr.mq_curmsgs>0) {
                    if(mq_receive(client_queue_id, s_msg, MAX_MSG_SIZE, &s_priority) == -1){
                        error("Problem with message receiving!\n");
                    }
                    if(handle_message(s_priority, s_msg)==1)
                        return 0;

                    printf("Enter message for client %d:", partner_id);
                    fflush(stdout);

                }
            }

        case DISCONNECT:
            printf("Client received DISCONNECT\n");

            if(mq_close(partner_queue_id) == -1){
                error("Problem with closing the server queue!\n");
            }

            partner_id = -1;
            partner_queue_id = -1;
            printf("Enter command:");
            fflush(stdout);
            return 1;

        case STOP:
            stop(0);

        case CHAT:
            printf("\nReceived message from %d: %s\n", partner_id, msg);
            break;

        default:
            printf("Message not known!\n");
            break;
    }

    return 0;
}

char random_char() {
    return rand() % ('z' - 'a' + 1) + 'a';
}

void get_client_queue_name() {

    client_queue_name[0] = '/';
    for(int i = 1; i < NAME_SIZE; i++)
        client_queue_name[i] = random_char();

}


int main(){

    if (atexit(bye) != 0) {
        error("Cannot set exit function!\n");
    }

    srand(time(NULL));
    get_client_queue_name();

    signal(SIGINT, stop);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    client_queue_id = mq_open(client_queue_name, O_RDONLY | O_CREAT, S_IRWXU | S_IXGRP | S_IWOTH, &attr);
    if(client_queue_id == -1){
        error("Problem with queue creation!\n");
    }


    server_queue_id = mq_open(SERVER_FILE_NAME, O_WRONLY);
    if(server_queue_id == -1){
        error("Problem with queue creation!\n");
    }

    if(mq_send(server_queue_id, client_queue_name, strlen(client_queue_name), INIT) == -1){
        error("Problem with sending a message with client id!\n");
    }

    unsigned int priority;
    char msg[MAX_MSG_SIZE];

    if(mq_receive(client_queue_id, msg, MAX_MSG_SIZE, &priority) == -1){
            error("Problem with message receiving!\n");
        }
    if(strcmp(msg, "-1")==0){
        perror("Server cannot register more clients!\n");
        return 1;
    }
    client_id = atoi(msg);

    char command[12];
    struct timeval tmo;
    fd_set readfds;

    printf("Enter command:");
    fflush(stdout);

    while(1){

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        tmo.tv_sec = 1;
        tmo.tv_usec = 0;

        switch (select(1, &readfds, NULL, NULL, &tmo)) {
            case -1:
                error("Error while using select!\n");
            case 1:
                scanf("%s", command);
                handle_command(command);
                printf("Enter command:");
                fflush(stdout);
                break;

        }
        if(mq_getattr(client_queue_id, &attr) == -1)
            error("Problem with getting queue info!\n");
        if(attr.mq_curmsgs>0) {
            if(mq_receive(client_queue_id, msg, MAX_MSG_SIZE, &priority) == -1){
                error("Problem with message receiving!\n");
            }
            handle_message(priority, msg);
        }

    }

    return 0;
}
