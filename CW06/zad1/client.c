//
// Created by werka on 4/20/20.
//

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "header.h"

int client_queue_id;
int server_queue_id;
int client_id;
int partner_queue_id = -1;
int partner_id = -1;

void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void bye(void){
    if(msgctl(client_queue_id, IPC_RMID, NULL)==-1){
        error("Problem with queue deletion!\n");
    }
}

void stop(int signum){
    message_buf sbuf;
    sbuf.client_id = client_id;
    if(partner_queue_id!=-1){
        sbuf.m_type = DISCONNECT;
        sbuf.partner_id = partner_id;
        if(msgsnd(partner_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
            error("Problem with sending a message!\n");
        }
        if(msgsnd(server_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
            error("Problem with sending a message!\n");
        }
    }

    sbuf.m_type = STOP;
    if(msgsnd(server_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
        error("Problem with sending a message!\n");
    }

    exit(0);
}

void handle_command(char * command){

    message_buf sbuf;
    sbuf.client_id = client_id;

    if(strcmp(command, "LIST")==0){
        printf("Handling LIST command\n");
        sbuf.m_type = LIST;
        if(msgsnd(server_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
            error("Problem with sending a message!\n");
        }
    }
    else if(strcmp(command, "CONNECT")==0){
        printf("Handling CONNECT command\n");
        int partner_id;
        scanf("%d", &partner_id);
        sbuf.m_type = CONNECT;
        sbuf.partner_id = partner_id;
        if(msgsnd(server_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
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

int handle_message(message_buf rbuf){

    message_buf sbuf;

    switch(rbuf.m_type){

        case CONNECT:
            printf("Client received CONNECT\n");
            partner_id = rbuf.partner_id;
            int handling_code;

            partner_queue_id = msgget(rbuf.queue_key, 0);

            char msg[MAX_MSG_SIZE];

            message_buf rbuf2;
            struct timeval tmo;
            fd_set readfds;
            printf("Enter message for client %d:", rbuf.partner_id);
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
                        scanf("%s", msg);
                        if(strcmp(msg, "DISCONNECT")==0){
                            sbuf.m_type = DISCONNECT;
                            sbuf.client_id = client_id;
                            sbuf.partner_id = partner_id;
                            if(msgsnd(server_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
                                error("Problem with sending a message!\n");
                            }
                            if(msgsnd(partner_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
                                error("Problem with sending a message!\n");
                            }
                            partner_queue_id = -1;
                            partner_id = -1;
                            printf("Enter command:");
                            fflush(stdout);
                            return 0;
                        }
                        sbuf.m_type = CHAT;
                        sbuf.partner_id = client_id;
                        memcpy(&sbuf.m_text, &msg, MAX_MSG_SIZE);
                        if(msgsnd(partner_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
                            error("Problem with sending a message!\n");
                        }
                        printf("Enter message for client %d:", rbuf.partner_id);
                        fflush(stdout);
                        break;
                }

                if(msgrcv(client_queue_id, &rbuf2, sizeof(rbuf2), -7, IPC_NOWAIT) != -1 && errno == ENOMSG){

                    handling_code = handle_message(rbuf2);
                    if(handling_code==1)
                        return 0;

                    printf("Enter message for client %d:", rbuf.partner_id);
                    fflush(stdout);

                }
            }

        case DISCONNECT:
            printf("Client received DISCONNECT\n");
            partner_id = -1;
            partner_queue_id = -1;
            printf("Enter command:");
            fflush(stdout);
            return 1;

        case STOP:
            stop(0);

        case CHAT:
            printf("\nReceived message from %d: %s\n", rbuf.partner_id, rbuf.m_text);
            break;

        default:
            printf("Message not known!\n");
            break;
    }

    return 0;
}


int main(){

    if (atexit(bye) != 0) {
        error("Cannot set exit function!\n");
    }

    signal(SIGINT, stop);
    srand(time(NULL));

    key_t client_queue_key = abs(ftok(getenv("HOME"), rand() % 254 + 2));
    client_queue_id = msgget(client_queue_key, IPC_CREAT | 0666);
    if(client_queue_id == -1){
        error("Problem with queue creation!");
    }

    key_t server_queue_key = ftok(getenv("HOME"), SERVER_GEN);
    server_queue_id = msgget(server_queue_key, 0);
    if(server_queue_id == -1){
        error("Problem with getting queue!");
    }

    message_buf sbuf;
    sbuf.queue_key = client_queue_key;
    sbuf.m_type = INIT;
    if(msgsnd(server_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
        error("Problem with sending a message!\n");
    }

    message_buf rbuf;
    if(msgrcv(client_queue_id, &rbuf, sizeof(rbuf), 0, 0) == -1){
        error("Problem with message receiving!\n");
    }
    client_id = rbuf.client_id;
    if(client_id == -1){
        perror("Server cannot register more clients!\n");
        return 1;
    }

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

        if(msgrcv(client_queue_id, &rbuf, sizeof(rbuf), -7, IPC_NOWAIT) != -1 && errno == ENOMSG){
            handle_message(rbuf);
        }

    }

    return 0;
}
