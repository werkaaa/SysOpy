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


void handler_increment(int sig, siginfo_t *info, void *ucontext) {
    sender_pid = info->si_pid;
    received_signals++;
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

    char * mode = argv[1];

    received_signals=0;
    waiting_for_incoming_signals=1;

    struct sigaction act_i, act_s;
    sigfillset(&act_i.sa_mask);
    sigfillset(&act_s.sa_mask);
    act_i.sa_flags = SA_SIGINFO;
    act_s.sa_flags = 0;
    act_i.sa_sigaction= handler_increment;
    act_s.sa_handler= handler_stop;

    if (strcmp(mode, "kill")==0 || strcmp(mode, "sigqueque")==0){

        //block rest of signals
        sigset_t mask;
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

       // signal(SIGUSR2, handler_stop);
    }
    else if (strcmp(mode, "sigrt")==0){

        //block rest of signals
        sigset_t mask;
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

       // signal(SIGRTMAX, handler_stop);
    }
    else{
        printf("Not known argument!\n");
        return 1;
    }

    while(waiting_for_incoming_signals==1){
        pause();
    }

    if (strcmp(mode, "kill")==0){

        for(int i = 0; i<received_signals; i++){
            kill(sender_pid, SIGUSR1);
        }
        kill(sender_pid, SIGUSR2);
    }
    else if(strcmp(mode, "sigqueque")==0){

        union sigval value;
        for(int i = 0; i<received_signals; i++){
            value.sival_int = i+1;
            sigqueue(sender_pid, SIGUSR1, value);
        }
        value.sival_int = received_signals;
        sigqueue(sender_pid, SIGUSR2, value);
    }
    else if (strcmp(mode, "sigrt")==0){

        for(int i = 0; i<received_signals; i++){
            kill(sender_pid, SIGRTMIN);
        }
        kill(sender_pid, SIGRTMAX);
    }
    return int main(int argc, char ** argv){0;
}

