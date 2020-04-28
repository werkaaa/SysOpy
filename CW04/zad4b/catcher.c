//
// Created by werka on 3/31/20.
//
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int received_signals;
int waiting_for_incoming_signals;
pid_t sender_pid;
int sending;
char * mode;
int wait;


void handler_increment(int sig, siginfo_t *info, void *ucontext) {
    if(sending==0) {
        sender_pid = info->si_pid;
        received_signals=received_signals+1;
        //printf("%d\n", received_signals);
        usleep(5);
        if(strcmp(mode, "kill")==0 || strcmp(mode, "sigrt")==0) {
            kill(sender_pid, sig);
        }
        else{
            union sigval value;
            value.sival_int = -1;
            sigqueue(sender_pid, SIGUSR1, value);
        }
    }
    else{
        wait = 0;
    }
}

void handler_stop(int  sig){
    waiting_for_incoming_signals=0;
}


int main(int argc, char ** argv){
    if(argc<=1){
        printf("You specified to few arguments!");
        return 1;
    }

    printf("%d\n", getpid());

    mode = argv[1];

    received_signals=0;
    waiting_for_incoming_signals=1;
    sending = 0;

    struct sigaction act_i, act_s;
    sigfillset(&act_i.sa_mask);
    sigfillset(&act_s.sa_mask);
    act_i.sa_flags = SA_SIGINFO;
    act_s.sa_flags = 0;
    act_i.sa_sigaction= handler_increment;
    act_s.sa_handler= handler_stop;

    sigset_t mask;

    if (strcmp(mode, "kill")==0 || strcmp(mode, "sigqueque")==0){

        //block rest of signals
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
            perror("Signal wasn't blocked!");

        if (sigaction(SIGUSR1, &act_i, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        if (sigaction(SIGUSR2, &act_s, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

    }
    else if (strcmp(mode, "sigrt")==0){

        //block rest of signals
        sigfillset(&mask);
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMAX);
        if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
            perror("Signal wasn't blocked!");

        if (sigaction(SIGRTMIN, &act_i, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        if (sigaction(SIGRTMAX, &act_s, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

    }
    else{
        printf("Not known argument!\n");
        return 1;
    }

    while(waiting_for_incoming_signals==1){
        pause();
    }
    sending=1;
    usleep(100);
    printf("%d\n", received_signals);


    if (strcmp(mode, "kill")==0){
        wait=0;
        for(int i = 0; i<received_signals; i++) {
            while (wait == 1){
                pause();

            }
            kill(sender_pid, SIGUSR1);
            wait=1;
        }
        kill(sender_pid, SIGUSR2);

    }
    else if(strcmp(mode, "sigqueque")==0){

        union sigval value;
        wait=0;
        for(int i = 0; i<received_signals; i++){
            while (wait == 1){
                pause();

            }
            value.sival_int = i+1;
            sigqueue(sender_pid, SIGUSR1, value);
            wait=1;

        }
        value.sival_int = received_signals;
        sigqueue(sender_pid, SIGUSR2, value);
    }
    else if (strcmp(mode, "sigrt")==0){

        wait=0;
        for(int i = 0; i<received_signals; i++){
            while(wait==1){
                pause();
            }
            kill(sender_pid, SIGRTMIN);
            wait=1;

        }
        kill(sender_pid, SIGRTMAX);
    }
    return 0;
}

