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
#include <semaphore.h>

int chairs_no, clients_no;
pthread_t now_shaved;
int* chairs;
pthread_t* clients;

int chairs_taken = 0;
int next_free_chair = 0;
int barber_sleeping = 0;
int clients_shaved = 0;
int next_client = 0;
int shaved_client_id;

sem_t * semaphores;

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
    int index = *((int *) index_);
    pthread_t client_id = pthread_self();
    while(1){
        pthread_mutex_lock(&mutex);
        if(barber_sleeping){
            printf("Budzę golibrodę; %ld\n", client_id);
            now_shaved = client_id;
            shaved_client_id = index;
            barber_sleeping = 0;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            if(sem_wait(&semaphores[index])==-1) error("Problem with waiting for the semaphore!");
            break;
        }
        else {
            if (chairs_taken < chairs_no) {
                chairs_taken++;
                chairs[next_free_chair] = index;
                next_free_chair = (next_free_chair + 1) % chairs_no;
                printf("Poczekalnia, wolne miejsca: %d; %ld\n", chairs_no - chairs_taken, client_id);
                pthread_mutex_unlock(&mutex);
                if(sem_wait(&semaphores[index])==-1) error("Problem with waiting for the semaphore!");
                break;
            } else {
                printf("Zajęte; %ld\n", client_id);
                pthread_mutex_unlock(&mutex);
                sleep(random_seconds());
            }
        }
    }

    printf("Jestem ogolony, wychodzę; %ld\n", client_id);
    pthread_exit((void *) 0);
}

void* barber_behaviour(void* arg) {
    int client_id;
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
            client_id = shaved_client_id;
            pthread_mutex_unlock(&mutex);
        }
        else{
            client_id = chairs[next_client];
            now_shaved = clients[client_id];
            next_client = (next_client + 1) % chairs_no;
            chairs_taken--;
            printf("Golibroda: czeka %d klientów, golę klienta %ld\n", chairs_taken, now_shaved);
            pthread_mutex_unlock(&mutex);
        }
        sleep(4*random_seconds());
        if(sem_post(&semaphores[client_id])==-1) error("Problem with semaphore incrementation!");
        clients_shaved++;

        usleep(10);
        if(clients_shaved==clients_no)
            break;

    }
    printf("Golibroda: wszyscy klienci ogoleni, kończę pracę na dziś\n");
    pthread_exit((void *) 0);
}

int main(int argc, char ** argv) {
    if (argc <= 2) {
        printf("You specified too few arguments!");
        return 1;
    }

    chairs_no = atoi(argv[1]);
    clients_no = atoi(argv[2]);

    clients = (pthread_t *)calloc(clients_no, sizeof(pthread_t));
    semaphores = (sem_t *)calloc(clients_no, sizeof(sem_t));

    for(int i = 0; i<clients_no; i++)
        if(sem_init(&semaphores[i], 0, 0)==-1) error("Cannot initialize semaphore!");

    chairs = (int *)calloc(chairs_no, sizeof(int));
    pthread_t barber_id;

    srand(time(0));

    if(pthread_create(&barber_id, NULL, barber_behaviour, NULL)!=0) error("Problem with thread creation!");

    for(int i = 0; i<clients_no; i++){
        sleep(random_seconds());
        int* i_ = (int*) malloc(sizeof(int));
        *i_ = i;
        if(pthread_create(&clients[i], NULL, client_behaviour, (void *)i_)!=0) error("Problem with thread creation!");
    }

    if(pthread_join(barber_id, NULL)!=0) error("Problem with thread!");

    for(int i = 0; i<clients_no; i++) {
        if(pthread_join(clients[i], NULL)!=0) error("Problem with thread!");
    }

    for(int i = 0; i<clients_no; i++)
        if(sem_destroy(&semaphores[i])==-1) error("Problem with semaphore destruction!");

    free(clients);
    free(chairs);
    free(semaphores);


    return 0;

}
