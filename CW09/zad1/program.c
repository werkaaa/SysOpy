//
// Created by werka on 5/11/20.
//
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

int chairs_no, clients_no;
pthread_t now_shaved;
pthread_t* chairs;

int chairs_taken = 0;
int next_free_chair = 0;
int barber_sleeping = 0;
int clients_shaved = 0;
int next_client = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;


void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}

int random_seconds(){
    return rand()%2+1;
}

void* client_behaviour(void* index_) {
    pthread_t client_id = pthread_self();
    while(1){
        pthread_mutex_lock(&mutex);
        if(barber_sleeping){
            printf("Budzę golibrodę; %ld\n", client_id);
            now_shaved = client_id;
            barber_sleeping = 0;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            break;
        }
        else {
            if (chairs_taken < chairs_no) {
                chairs_taken++;
                chairs[next_free_chair] = client_id;
                next_free_chair = (next_free_chair + 1) % chairs_no;
                printf("Poczekalnia, wolne miejsca: %d; %ld\n", chairs_no - chairs_taken, client_id);
                pthread_mutex_unlock(&mutex);
                break;
            } else {
                printf("Zajęte; %ld\n", client_id);
                pthread_mutex_unlock(&mutex);
                sleep(random_seconds());
            }
        }

    }

    pthread_exit((void *) 0);
}

void* barber_behaviour(void* index_) {

    while(1){
        pthread_mutex_lock(&mutex);
        if(chairs_taken==0){
            pthread_mutex_unlock(&mutex);
            barber_sleeping = 1;
            printf("Golibroda: idę spać\n");
            pthread_mutex_lock(&mutex);
            while (barber_sleeping) {
                pthread_cond_wait(&cond, &mutex);
            }
            printf("Golibroda: właśnie obudzony, golę klienta %ld\n", now_shaved);
            pthread_mutex_unlock(&mutex);
        }
        else{
            now_shaved = chairs[next_client];
            next_client = (next_client + 1) % chairs_no;
            chairs_taken--;
            printf("Golibroda: czeka %d klientów, golę klienta %ld\n", chairs_taken, now_shaved);
            pthread_mutex_unlock(&mutex);
        }
        sleep(4*random_seconds());
        clients_shaved++;

        if(clients_shaved==clients_no)
            break;

    }

    pthread_exit((void *) 0);
}

int main(int argc, char ** argv) {
    if (argc <= 2) {
        printf("You specified too few arguments!");
        return 1;
    }

    chairs_no = atoi(argv[1]);
    clients_no = atoi(argv[2]);

    pthread_t* clients = (pthread_t *)calloc(clients_no, sizeof(pthread_t));
    chairs = (pthread_t *)calloc(chairs_no, sizeof(pthread_t));
    pthread_t barber_id;

    srand(time(0));

    if(pthread_create(&barber_id, NULL, barber_behaviour, NULL)!=0) error("Problem with thread creation!\n");

    for(int i = 0; i<clients_no; i++){
        sleep(random_seconds());
        if(pthread_create(&clients[i], NULL, client_behaviour, NULL)!=0) error("Problem with thread creation!\n");
    }

    if(pthread_join(barber_id, NULL)!=0) error("Problem with thread!\n");

    for(int i = 0; i<clients_no; i++) {
        if(pthread_join(clients[i], NULL)!=0) error("Problem with thread!\n");
    }



    return 0;

}
