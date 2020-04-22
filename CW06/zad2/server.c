//
// Created by werka on 4/20/20.
//

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "header.h"

mqd_t server_queue_id;
char clients_queues [MAX_CLIENTS][MAX_MSG_SIZE];
mqd_t clients_queues_id [MAX_CLIENTS];
int clients [MAX_CLIENTS];
int clients_counter = 0;

void error(char* msg) {
    printf("Error: %s", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}

int get_next_client_id(){
    if(clients_counter < MAX_CLIENTS){
        clients_counter++;
        for(int i = 0; i<MAX_CLIENTS; i++){
            if(clients_queues[i][0]=='0') {

                return i;
            }
        }
    }
    return -1;
}

void bye(void){
    if(mq_unlink(SERVER_FILE_NAME)==-1){
        error("Problem with queue deletion!\n");
    }
}

void unregister_client(int client_id){
    if(mq_close(clients_queues_id[client_id]) == -1){
        error("Problem with closing the queue!\n");
    }
    clients[client_id] = 0;
    memset(clients_queues[client_id], '\0', MAX_MSG_SIZE);
    clients_queues[client_id][0] = '0';
    clients_queues_id[client_id] = -1;
    clients_counter--;
}

void handle_message(unsigned int priority, char * msg){
    int client_id;
    int partner_id;
    mqd_t client_queue_id;
    unsigned int s_priority;
    char s_msg[MAX_MSG_SIZE];
    char* tok;

    switch(priority){
        case INIT:
            printf("Server received INIT\n");
            client_queue_id = mq_open(msg, O_WRONLY);
            if(client_queue_id == -1)
                error("Problem with connection!\n");
            client_id = get_next_client_id();
            s_priority = INIT;
            if(client_id == -1){
                printf("Cannot register more clients!");
                fflush(stdout);
                strcpy(s_msg, "-1");
            }
            else {
                clients[client_id] = 1;
                strcpy(clients_queues[client_id], msg);
                snprintf(s_msg, MAX_MSG_SIZE, "%d", client_id);
            }
            if(mq_send(client_queue_id, s_msg, MAX_MSG_SIZE, s_priority) == -1){
                error("Problem with sending a message with client id!\n");
            }
            clients_queues_id[client_id] = client_queue_id;
            break;

        case LIST:
            printf("Server received LIST\n");
            for(int i = 0; i<MAX_CLIENTS; i++){
                if(clients_queues[i][0]!='0')
                    printf("Client id: %d, Client queue name %s, Client available %d\n", i, clients_queues[i], clients[i]);
            }
            break;

        case CONNECT:
            printf("Server received CONNECT\n");

            tok = strtok(msg, " ");
            client_id = atoi(tok);
            tok = strtok(NULL, " ");
            partner_id = atoi(tok);

            if(clients[partner_id] == 0)
                break;

            clients[client_id] = 0;
            clients[partner_id] = 0;

            s_priority = CONNECT;

            // do proszącego o połączenie
            snprintf(s_msg, MAX_MSG_SIZE, "%d %d %s ", client_id, partner_id, clients_queues[partner_id]);
            if(mq_send(clients_queues_id[client_id], s_msg, sizeof(s_msg), s_priority) == -1){
                error("Problem with sending a message!\n");
            }

            // do potencjalnego rozmówcy
            snprintf(s_msg, MAX_MSG_SIZE, "%d %d %s ", partner_id, client_id, clients_queues[client_id]);
            if(mq_send(clients_queues_id[partner_id], s_msg, sizeof(s_msg), s_priority) == -1){
                error("Problem with sending a message!\n");
            }

            break;

        case DISCONNECT:
            printf("Server received DISCONNECT\n");

            tok = strtok(msg, " ");
            client_id = atoi(tok);
            tok = strtok(NULL, " ");
            partner_id = atoi(tok);

            clients[client_id] = 1;
            clients[partner_id] = 1;
            break;

        case STOP:
            printf("Server received STOP\n");
            client_id = atoi(msg);
            unregister_client(client_id);
            break;

        default:
            printf("Server received not known message!\n");
            break;
    }
}


void stop(int signum){
    unsigned int priority;
    char msg[MAX_MSG_SIZE];
    int client_id;
    for(int i = 0; i<MAX_CLIENTS; i++) {

        if(clients_queues[i][0]!='0') {
            priority = STOP;

            if (mq_send(clients_queues_id[i], msg, strlen(msg), priority) == -1) {
                error("Problem with sending a closing message!\n");
            }

            priority = CHAT; // dla pewności
            if (mq_receive(server_queue_id, msg, MAX_MSG_SIZE, &priority) == -1) {
                error("Problem with closing message receiving!\n");
            }

            client_id = atoi(msg);
            while(priority!=STOP || client_id!=i){
                if (mq_receive(server_queue_id, msg, MAX_MSG_SIZE, &priority) == -1) {
                    error("Problem with message receiving!\n");
                }
                client_id = atoi(msg);
            }
            unregister_client(client_id);
        }
    }
    exit(0);
}


int main(){

    if (atexit(bye) != 0) {
        error("Cannot set exit function!\n");
    }

    signal(SIGINT, stop);

    for(int i = 0; i<MAX_CLIENTS; i++) clients[i] = 0;
    for(int i = 0; i<MAX_CLIENTS; i++) clients_queues[i][0] = '0';
    for(int i = 0; i<MAX_CLIENTS; i++) clients_queues_id[i] = -1;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    server_queue_id = mq_open(SERVER_FILE_NAME, O_CREAT | O_RDONLY, S_IRWXU | S_IXGRP | S_IWOTH, &attr);
    if(server_queue_id == -1){
        error("Problem with queue creation!\n");
    }

    unsigned int priority;
    char * msg = (char *)calloc(MAX_MSG_SIZE, sizeof(char));
    while(1){
        if(mq_receive(server_queue_id, msg, MAX_MSG_SIZE, &priority) == -1){
            error("Problem with message receiving!\n");
        }
        handle_message(priority, msg);
        memset(msg, '\0', MAX_MSG_SIZE);
    }

    return 0;
}
