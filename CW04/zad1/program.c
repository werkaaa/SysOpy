//
// Created by werka on 3/30/20.
//
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int paused = 0;

void handle_sigtstp(int sig){
    if(!paused){
        paused=1;
        printf("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
    }
    else{
        paused=0;
    }
}


void handle_sigint(int sig)
{
    printf("\nOdebrano sygnał SIGINT\n");
    exit(0);
}


int main(){

    struct sigaction act;

    act.sa_handler = handle_sigtstp;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGTSTP, &act, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }

    signal(SIGINT, handle_sigint);

    while(1){
        if(!paused) {
            system("ls");
            sleep(1);
        }
        else{
            pause();
        }
    }

    return 0;
}