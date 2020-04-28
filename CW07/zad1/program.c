//
// Created by werka on 4/27/20.
//
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "header.h"

int sem_id;
int shm_id;

void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}

long int get_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);

    double time_in_mill =
            (tv.tv_sec) * 1000 + (long int)((tv.tv_usec) / 1000);
    return time_in_mill;
}

void receive_order(){

    struct sembuf* buf = (struct sembuf*) calloc(2, sizeof(struct sembuf));

    buf[0].sem_num = 0;
    buf[0].sem_op = -1;
    buf[0].sem_flg = 0;

    buf[1].sem_num = 2;
    buf[1].sem_op = 1;
    buf[1].sem_flg = 0;

    buf[2].sem_num = 6;
    buf[2].sem_op = -1;
    buf[2].sem_flg = 0;

    if(semop(sem_id, buf, 3)==-1) error("Problem with semop!\n");

    int id = semctl(sem_id, 1, GETVAL, NULL);
    if(id==-1) error("Problem with semctl!\n");

    int to_prepare = semctl(sem_id, 2, GETVAL, NULL);
    if(to_prepare==-1) error("Problem with semctl!\n");

    int to_send = semctl(sem_id, 4, GETVAL, NULL);
    if(to_send==-1) error("Problem with semctl!\n");

    orders* ord = shmat(shm_id, NULL, 0);
    int order_size = rand()%50+1;
    ord->parcels[id] = order_size;
    printf("(%d %ld) ID w tablicy: %d Dodałem liczbę: %d. Liczba zamówień do przygotowania: %d. Liczba zamówień do wysłania: %d\n", getpid(), get_time(), id, order_size, to_prepare, to_send);
    shmdt(ord);

    union semun arg;
    arg.val = (id+1)%MAX_ORDERS;
    if(semctl(sem_id, 1, SETVAL, arg)==-1) error("Problem with semctl!\n");

    buf[0].sem_num = 6;
    buf[0].sem_op = 1;
    buf[0].sem_flg = 0;

    if(semop(sem_id, buf, 1)==-1) error("Problem with semop!\n");
    free(buf);
}

void pack_order(){

    struct sembuf* buf = (struct sembuf*) calloc(2, sizeof(struct sembuf));

    buf[0].sem_num = 2;
    buf[0].sem_op = -1;
    buf[0].sem_flg = 0;

    buf[1].sem_num = 4;
    buf[1].sem_op = 1;
    buf[1].sem_flg = 0;

    buf[2].sem_num = 6;
    buf[2].sem_op = -1;
    buf[2].sem_flg = 0;

    if(semop(sem_id, buf, 3)==-1) error("Problem with semop!\n");

    int id = semctl(sem_id, 3, GETVAL, NULL);
    if(id==-1) error("Problem with semctl!\n");

    int to_prepare = semctl(sem_id, 2, GETVAL, NULL);
    if(to_prepare==-1) error("Problem with semctl!\n");

    int to_send = semctl(sem_id, 4, GETVAL, NULL);
    if(to_send==-1) error("Problem with semctl!\n");

    orders* ord = shmat(shm_id, NULL, 0);
    ord->parcels[id] *= 2;
    int order_size = ord->parcels[id];
    printf("(%d %ld) ID w tablicy: %d Przygotowałem zamówienie o wielkości: %d. Liczba zamówień do przygotowania: %d. Liczba zamówień do wysłania: %d\n", getpid(), get_time(), id, order_size, to_prepare, to_send);
    shmdt(ord);

    union semun arg;
    arg.val = (id+1)%MAX_ORDERS;
    if(semctl(sem_id, 3, SETVAL, arg)==-1) error("Problem with semctl!\n");

    buf[0].sem_num = 6;
    buf[0].sem_op = 1;
    buf[0].sem_flg = 0;

    if(semop(sem_id, buf, 1)==-1) error("Problem with semop!\n");
}

void send_order(){

    struct sembuf* buf = (struct sembuf*) calloc(2, sizeof(struct sembuf));

    buf[0].sem_num = 4;
    buf[0].sem_op = -1;
    buf[0].sem_flg = 0;

    buf[1].sem_num = 0;
    buf[1].sem_op = 1;
    buf[1].sem_flg = 0;

    buf[2].sem_num = 6;
    buf[2].sem_op = -1;
    buf[2].sem_flg = 0;

    if(semop(sem_id, buf, 3)==-1) error("Problem with semop!\n");

    int id = semctl(sem_id, 5, GETVAL, NULL);
    if(id==-1) error("Problem with semctl!\n");

    int to_prepare = semctl(sem_id, 2, GETVAL, NULL);
    if(to_prepare==-1) error("Problem with semctl!\n");

    int to_send = semctl(sem_id, 4, GETVAL, NULL);
    if(to_send==-1) error("Problem with semctl!\n");

    orders* ord = shmat(shm_id, NULL, 0);
    ord->parcels[id] *= 3;
    int order_size = ord->parcels[id];
    printf("(%d %ld) ID w tablicy: %d Wysłałem zamówienie o wielkości: %d. Liczba zamówień do przygotowania: %d. Liczba zamówień do wysłania: %d\n", getpid(), get_time(), id, order_size, to_prepare, to_send);
    shmdt(ord);

    union semun arg;
    arg.val = (id+1)%MAX_ORDERS;
    if(semctl(sem_id, 5, SETVAL, arg)==-1) error("Problem with semctl!\n");

    buf[0].sem_num = 6;
    buf[0].sem_op = 1;
    buf[0].sem_flg = 0;

    if(semop(sem_id, buf, 1)==-1) error("Problem with semop!\n");
}


int main(int argc, char ** argv){
    if(argc<=1){
        printf("Specify worker type!\n");
        return 1;
    }

    key_t sem_key = ftok(getenv("HOME"), 0);
    sem_id = semget(sem_key, 0, 0);
    if(sem_id == -1) error("Cannot access semafors set!\n");


    key_t shm_key = ftok(getenv("HOME"), 1);
    shm_id = shmget(shm_key, 0, 0);
    if(shm_id == -1) error("Cannot access shared memory segment!\n");

    int worker_type = atoi(argv[1]);

    switch(worker_type){
        case 1:
            srand(time(NULL));
            while(1){
                receive_order();
            }
        case 2:
            while(1){
                pack_order();
            }
        case 3:
            while(1){
                send_order();
            }
        default:
            printf("Worker type not known!\n");
            return 1;
    }

    return 0;
}