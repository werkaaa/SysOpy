//
// Created by werka on 4/27/20.
//
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "header.h"

int shm_id;

sem_t * to_receive;
sem_t * id_to_receive;
sem_t * to_prepare;
sem_t * id_to_prepare;
sem_t * to_send;
sem_t * id_to_send;
sem_t * mem_access;

void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}


void bye(void){
    if (sem_close(to_receive) == -1) error("Problem with semaphore deletion!");
    if (sem_close(id_to_receive) == -1) error("Problem with semaphore deletion!");
    if (sem_close(to_prepare) == -1) error("Problem with semaphore deletion!");
    if (sem_close(id_to_prepare) == -1) error("Problem with semaphore deletion!");
    if (sem_close(to_send) == -1) error("Problem with semaphore deletion!");
    if (sem_close(id_to_send) == -1) error("Problem with semaphore deletion!");
    if (sem_close(mem_access) == -1) error("Problem with semaphore deletion!");
}

long int get_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);

    double time_in_mill =
            (tv.tv_sec) * 1000 + (long int)((tv.tv_usec) / 1000);
    return time_in_mill;
}

void receive_order(){

        if (sem_wait(mem_access) == -1) error("Problem with sem_wait!");
        if (sem_trywait(to_receive) == -1 && errno == EAGAIN) {
            if (sem_post(mem_access) == -1) error("Problem with sem_post!");
            return;
        }
        if (sem_post(to_prepare) == -1) error("Problem with sem_post!");

        int id;
        if (sem_getvalue(id_to_receive, &id) == -1) error("Problem with getting semaphore value!");
        id = id % MAX_ORDERS;

        int to_prepare_num;
        if (sem_getvalue(to_prepare, &to_prepare_num) == -1) error("Problem with getting semaphore value!");

        int to_send_num;
        if (sem_getvalue(to_send, &to_send_num) == -1) error("Problem with getting semaphore value!");

        orders *ord = mmap(NULL, sizeof(orders), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, shm_id, 0);
        if (ord == (void *) -1) error("Problem with mmap!");
        int order_size = rand() % 50 + 1;
        ord->parcels[id] = order_size;
        printf("(%d %ld) ID w tablicy: %d Dodałem liczbę: %d. Liczba zamówień do przygotowania: %d. Liczba zamówień do wysłania: %d\n",
               getpid(), get_time(), id, order_size, to_prepare_num, to_send_num);
        if (munmap(ord, sizeof(orders)) == -1) error("Problem with munmap!");

        if (sem_post(id_to_receive) == -1) error("Problem with sem_post!");
        if (sem_post(mem_access) == -1) error("Problem with sem_post!");

}

void pack_order(){
        if (sem_wait(mem_access) == -1) error("Problem with sem_wait!");
        if (sem_trywait(to_prepare) == -1 && errno == EAGAIN) {
            if (sem_post(mem_access) == -1) error("Problem with sem_post!");
            return;
        }

        if (sem_post(to_send) == -1) error("Problem with sem_post!");

        int id;
        if (sem_getvalue(id_to_prepare, &id) == -1) error("Problem with getting semaphore value!");
        id = id % MAX_ORDERS;

        int to_prepare_num;
        if (sem_getvalue(to_prepare, &to_prepare_num) == -1) error("Problem with getting semaphore value!");

        int to_send_num;
        if (sem_getvalue(to_send, &to_send_num) == -1) error("Problem with getting semaphore value!");

        orders *ord = mmap(NULL, sizeof(orders), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, shm_id, 0);
        if (ord == (void *) -1) error("Problem with mmap!");
        ord->parcels[id] *= 2;
        int order_size = ord->parcels[id];
        printf("(%d %ld) ID w tablicy: %d Przygotowałem zamówienie o wielkości: %d. Liczba zamówień do przygotowania: %d. Liczba zamówień do wysłania: %d\n",
               getpid(), get_time(), id, order_size, to_prepare_num, to_send_num);
        if (munmap(ord, sizeof(orders)) == -1) error("Problem with munmap!");

        if (sem_post(id_to_prepare) == -1) error("Problem with sem_post!");
        if (sem_post(mem_access) == -1) error("Problem with sem_post!");
}

void send_order(){

        if (sem_wait(mem_access) == -1) error("Problem with sem_wait!");
        if (sem_trywait(to_send) == -1 && errno == EAGAIN) {
            if (sem_post(mem_access) == -1) error("Problem with sem_post!");
            return;
        }
        if (sem_post(to_receive) == -1) error("Problem with sem_post!");

        int id;
        if (sem_getvalue(id_to_send, &id) == -1) error("Problem with getting semaphore value!");
        id = id % MAX_ORDERS;

        int to_prepare_num;
        if (sem_getvalue(to_prepare, &to_prepare_num) == -1) error("Problem with getting semaphore value!");

        int to_send_num;
        if (sem_getvalue(to_send, &to_send_num) == -1) error("Problem with getting semaphore value!");

        orders *ord = mmap(NULL, sizeof(orders), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, shm_id, 0);
        if (ord == (void *) -1) error("Problem with mmap!");
        ord->parcels[id] *= 3;
        int order_size = ord->parcels[id];
        printf("(%d %ld) ID w tablicy: %d Wysłałem zamówienie o wielkości: %d. Liczba zamówień do przygotowania: %d. Liczba zamówień do wysłania: %d\n",
               getpid(), get_time(), id, order_size, to_prepare_num, to_send_num);
        if (munmap(ord, sizeof(orders)) == -1) error("Problem with munmap!");

        if (sem_post(id_to_send) == -1) error("Problem with sem_post!");
        if (sem_post(mem_access) == -1) error("Problem with sem_post!");
}

void open_semaphores(){
    to_receive = sem_open(SEM_TO_RECEIVE, O_RDWR);
    if(to_receive == SEM_FAILED) error("Cannot create semaphore!");

    id_to_receive = sem_open(SEM_ID_TO_RECEIVE, O_RDWR);
    if(id_to_receive == SEM_FAILED) error("Cannot create semaphore!");

    to_prepare = sem_open(SEM_TO_PREPARE, O_RDWR);
    if(to_prepare == SEM_FAILED) error("Cannot create semaphore!");

    id_to_prepare = sem_open(SEM_ID_TO_PREPARE, O_RDWR);
    if(id_to_prepare == SEM_FAILED) error("Cannot create semaphore!");

    to_send = sem_open(SEM_TO_SEND, O_CREAT, O_RDWR);
    if(to_send == SEM_FAILED) error("Cannot create semaphore!");

    id_to_send = sem_open(SEM_ID_TO_SEND, O_CREAT, O_RDWR);
    if(id_to_send == SEM_FAILED) error("Cannot create semaphore!");

    mem_access = sem_open(SEM_MEM_ACCESS, O_CREAT, O_RDWR);
    if(mem_access == SEM_FAILED) error("Cannot create semaphore!");

}


int main(int argc, char ** argv){
    if(argc<=1){
        printf("Specify worker type!\n");
        return 1;
    }

    int worker_type = atoi(argv[1]);

    if (atexit(bye) != 0) {
        error("Cannot set exit function!\n");
    }

    open_semaphores();

    shm_id = shm_open(ORDERS, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if(shm_id == -1) error("Cannot access shared memory!");


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