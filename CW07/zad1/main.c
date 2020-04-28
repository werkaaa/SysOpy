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
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

#include "header.h"

int sem_id;
int shm_id;

pid_t children[W_1+W_2+W_3];

void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}

void bye(void){
    if (semctl(sem_id, 0, IPC_RMID, NULL) == -1) error("Problem with semaphore deletion!");
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) error("Problem with shared memory deletion!");
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

    key_t sem_key = ftok(getenv("HOME"), 0);
    sem_id = semget(sem_key, 7, IPC_CREAT | 0666);
    if(sem_id == -1) error("Cannot create semafors set!");

    //0 - liczba wolnych miejsc
    //1 - indeks kolejnego wolnego miejsca
    //2 - liczba zamówień do przygotowania
    //3 - indeks pierwszego zamówienia do przygotowania
    //4 - liczba zamówień do wysłania
    //5 - indeks pierwszego zamówienia do wysłania
    //6 - czy pamięć jest czytana

    union semun arg;
    arg.val = MAX_ORDERS;
    if(semctl(sem_id, 0, SETVAL, arg)==-1) error("Problem with semctl!");
    arg.val = 0;
    for(int i = 1; i<6; i++)
        if(semctl(sem_id, i, SETVAL, arg)==-1) error("Problem with semctl!");

    arg.val = 1;
    if(semctl(sem_id, 6, SETVAL, arg)==-1) error("Problem with semctl!");

    key_t shm_key = ftok(getenv("HOME"), 1);
    shm_id = shmget(shm_key, sizeof(orders), IPC_CREAT | 0666);
    if(shm_id == -1) error("Cannot access shared memory segment!");

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
