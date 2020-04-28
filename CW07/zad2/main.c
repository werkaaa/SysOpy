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
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

#include "header.h"

pid_t children[W_1+W_2+W_3];

void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}

void bye(void){
    if (sem_unlink(SEM_TO_RECEIVE) == -1) error("Problem with semaphore deletion!");
    if (sem_unlink(SEM_ID_TO_RECEIVE) == -1) error("Problem with semaphore deletion!");
    if (sem_unlink(SEM_TO_PREPARE) == -1) error("Problem with semaphore deletion!");
    if (sem_unlink(SEM_ID_TO_PREPARE) == -1) error("Problem with semaphore deletion!");
    if (sem_unlink(SEM_TO_SEND) == -1) error("Problem with semaphore deletion!");
    if (sem_unlink(SEM_ID_TO_SEND) == -1) error("Problem with semaphore deletion!");
    if (sem_unlink(SEM_MEM_ACCESS) == -1) error("Problem with semaphore deletion!");
    if (shm_unlink(ORDERS) == -1) error("Problem with shared memory deletion!");
}

void handle_sigint(int sig)
{
    for(int i = 0; i<W_1+W_2+W_3; i++)
        kill(children[i], SIGINT);

    printf("Exit\n");
    exit(0);
}

int main(){

    if (atexit(bye) != 0) {
        error("Cannot set exit function!\n");
    }
    signal(SIGINT, handle_sigint);

    sem_t * sem = sem_open(SEM_TO_RECEIVE, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, MAX_ORDERS);
    if(sem == SEM_FAILED) error("Cannot create semaphore!");
    sem_close(sem);

    sem = sem_open(SEM_ID_TO_RECEIVE, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if(sem == SEM_FAILED) error("Cannot create semaphore!");
    sem_close(sem);

    sem = sem_open(SEM_TO_PREPARE, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if(sem == SEM_FAILED) error("Cannot create semaphore!");
    sem_close(sem);

    sem = sem_open(SEM_ID_TO_PREPARE, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if(sem == SEM_FAILED) error("Cannot create semaphore!");
    sem_close(sem);

    sem = sem_open(SEM_TO_SEND, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if(sem == SEM_FAILED) error("Cannot create semaphore!");
    sem_close(sem);

    sem = sem_open(SEM_ID_TO_SEND, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if(sem == SEM_FAILED) error("Cannot create semaphore!");
    sem_close(sem);

    sem = sem_open(SEM_MEM_ACCESS, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 1);
    if(sem == SEM_FAILED) error("Cannot create semaphore!");
    sem_close(sem);

    //0 - liczba wolnych miejsc
    //1 - indeks kolejnego wolnego miejsca
    //2 - liczba zamówień do przygotowania
    //3 - indeks pierwszego zamówienia do przygotowania
    //4 - liczba zamówień do wysłania
    //5 - indeks pierwszego zamówienia do wysłania
    //6 - czy pamięć jest czytana


    int shm_id = shm_open(ORDERS, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if(shm_id==-1) error("Problem with shared memory creation!");
    if(ftruncate(shm_id, sizeof(orders))==-1) error("Cannot set shared memory size!");

    pid_t child_pid;
    for(int i = 0; i<W_1; i++){
        child_pid = fork();
        if(child_pid<0) error("Problem with fork!");
        if(child_pid == 0) execlp("./program", "program", "1", NULL);
        else children[i] = child_pid;
    }
    for(int i = W_1; i<W_1+W_2; i++){
        child_pid = fork();
        if(child_pid<0) error("Problem with fork!");
        if(child_pid == 0) execlp("./program", "program", "2", NULL);
        else children[i] = child_pid;
    }
    for(int i = W_1+W_2; i<W_1+W_2+W_3; i++){
        child_pid = fork();
        if(child_pid<0) error("Problem with fork!");
        if(child_pid == 0) execlp("./program", "program", "3", NULL);
        else children[i] = child_pid;
    }

    for(int i = 0; i < W_1 + W_2 + W_3; i++) {
        wait(NULL);
    }
}
