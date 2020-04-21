//
// Created by werka on 4/20/20.
//
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include "header.h"

int server_queue_id;
key_t clients_queues [MAX_CLIENTS];
int clients [MAX_CLIENTS];
int clients_counter = 0;

void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

int get_next_client_id(){
    if(clients_counter < MAX_CLIENTS){
        clients_counter++;
        for(int i = 0; i<MAX_CLIENTS; i++){
            if(clients_queues[i]==-1)
                return i;
        }
    }
    return -1;
}

void bye(void){
    if(msgctl(server_queue_id, IPC_RMID, NULL)==-1){
        error("Problem with queue deletion!\n");
    }
}


void handle_message(message_buf rbuf){
    int client_id;
    int partner_id;
    int client_queue_id;
    message_buf sbuf;

    switch(rbuf.m_type){
        case INIT:
            printf("Server received INIT\n");

            client_queue_id = msgget(rbuf.queue_key, 0); //na pewno ta flaga?
            client_id = get_next_client_id();
            sbuf.m_type = INIT;
            if(client_id == -1){
                printf("Cannot register more clients!");
                fflush(stdout);
                sbuf.client_id = -1;
            }
            else {

                if (client_queue_id == -1) {
                    printf("Problem with connection! %d\n", errno);
                    clients_counter--;
                    break;
                }
                clients[client_id] = 1;
                clients_queues[client_id] = rbuf.queue_key;


                sbuf.client_id = client_id;
            }
            if(msgsnd(client_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
                error("Problem with sending a message with client id!\n");
            }
            break;

        case LIST:
            printf("Server received LIST\n");
            for(int i = 0; i<MAX_CLIENTS; i++){
                if(clients_queues[i]!=-1)
                    printf("Client id: %d, Client queue key %d, Client available %d\n", i, clients_queues[i], clients[i]);
            }
            break;

        case CONNECT:
            printf("Server received CONNECT\n");
            partner_id = rbuf.partner_id;
            if(clients[partner_id] == 0)
                break;

            client_id = rbuf.client_id;

            clients[client_id] = 0;
            clients[partner_id] = 0;

            sbuf.m_type = CONNECT;

            // do proszącego o połączenie
            sbuf.client_id = client_id;
            sbuf.partner_id = partner_id;
            sbuf.queue_key = clients_queues[partner_id];

            client_queue_id = msgget(clients_queues[client_id], 0);
            if(msgsnd(client_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
                error("Problem with sending a message!\n");
            }

            // do potencjalnego rozmówcy
            sbuf.client_id = partner_id;
            sbuf.partner_id = client_id;
            sbuf.queue_key = clients_queues[client_id];

            client_queue_id = msgget(clients_queues[partner_id], 0);
            if(msgsnd(client_queue_id, &sbuf, sizeof(sbuf), 0) == -1){
                error("Problem with sending a message!\n");
            }
            break;

        case DISCONNECT:
            printf("Server received DISCONNECT\n");
            clients[rbuf.client_id] = 1;
            clients[rbuf.partner_id] = 1;
            break;

        case STOP:
            printf("Server received STOP\n");
            client_id = rbuf.client_id;
            clients[client_id] = 0;
            clients_queues[client_id] = -1;
            clients_counter--;
            break;

        default:

            break;
    }
}

void stop(int signum){
    message_buf sbuf;
    message_buf rbuf;
    int client_queue_id;
    for(int i = 0; i<MAX_CLIENTS; i++) {

        if(clients_queues[i]!=-1) {
            sbuf.m_type = STOP;

            client_queue_id = msgget(clients_queues[i], 0);
            if(client_queue_id == -1){
                error("Problem with connection!\n");
            }

            if (msgsnd(client_queue_id, &sbuf, sizeof(sbuf), 0) == -1) {
                error("Problem with sending a message!\n");
            }

            rbuf.m_type = CHAT; // dla pewności
            if (msgrcv(server_queue_id, &rbuf, sizeof(rbuf), 0, 0) == -1) {
                error("Problem with message receiving!\n");
            }
            while(rbuf.m_type!=STOP || rbuf.client_id!=i){
                if (msgrcv(server_queue_id, &rbuf, sizeof(rbuf), 0, 0) == -1) {
                    error("Problem with message receiving!\n");
                }
            }
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
    for(int i = 0; i<MAX_CLIENTS; i++) clients_queues[i] = -1;

    key_t queue_key = ftok(getenv("HOME"), SERVER_GEN); //małą liczbę trzeba z pliku
    server_queue_id = msgget(queue_key, IPC_CREAT | 0666);
    if(server_queue_id == -1){
        error("Problem with queue creation!\n");
    }

    message_buf rbuf;
    while(1){
        if(msgrcv(server_queue_id, &rbuf, sizeof(rbuf), -6, 0) == -1){
            error("Problem with message receiving!\n");
        }
        handle_message(rbuf);
    }

    return 0;
}
