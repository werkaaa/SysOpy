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
int N;
int max_signal_number;

void handler_increment_sigqueque(int sig, siginfo_t *info, void *ucontext) {
    received_signals++;
    if(info->si_int>max_signal_number){
        max_signal_number = info->si_int;
    }
}

void handler_stop_sigqueque(int sig, siginfo_t *info, void *ucontext){
    waiting_for_incoming_signals=0;
    int send_by_catcher = info->si_int;
    printf("Signals sent %d\nSignals received %d\nMax signal number received %d\nSignals sent by catcher %d\n", N, received_signals, max_signal_number, send_by_catcher);
}

void handler_increment(int sig){
    received_signals++;
}

void handler_stop(int  sig){
    waiting_for_incoming_signals=0;
    printf("Signals sent %d\nSignals received %d\n", N, received_signals);
}

int main(int argc, char ** argv){
    if(argc<=3){
        printf("You specified to few arguments!");
        return 1;
    }
    int catcher_pid = atoi(argv[1]);
    N = atoi(argv[2]);
    char* mode = argv[3];

    received_signals=0;
    waiting_for_incoming_signals=1;

    if (strcmp(mode, "kill")==0){
        struct sigaction act_i, act_s;
        sigfillset(&act_i.sa_mask);
        sigfillset(&act_s.sa_mask);
        act_i.sa_flags = 0;
        act_s.sa_flags = 0;
        act_i.sa_handler= handler_increment;
        act_s.sa_handler= handler_stop;

        if (sigaction(SIGUSR1, &act_i, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        if (sigaction(SIGUSR2, &act_s, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

//        signal(SIGUSR1, handler_increment);
//        signal(SIGUSR2, handler_stop);

        for(int i = 0; i<N; i++){
            kill(catcher_pid, SIGUSR1);
        }
        kill(catcher_pid, SIGUSR2);
    }
    else if(strcmp(mode, "sigqueque")==0){
        struct sigaction act_i, act_s;
        sigfillset(&act_i.sa_mask);
        sigfillset(&act_s.sa_mask);
        act_i.sa_flags = SA_SIGINFO;
        act_s.sa_flags = SA_SIGINFO;
        act_i.sa_sigaction= handler_increment_sigqueque;
        act_s.sa_sigaction= handler_stop_sigqueque;

        if (sigaction(SIGUSR1, &act_i, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        if (sigaction(SIGUSR2, &act_s, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        union sigval value;
        for(int i = 0; i<N; i++){
            value.sival_int = i+1;
            sigqueue(catcher_pid, SIGUSR1, value);
        }
        value.sival_int = N;
        sigqueue(catcher_pid, SIGUSR2, value);
    }
    if (strcmp(mode, "sigrt")==0){
        struct sigaction act_i, act_s;
        sigfillset(&act_i.sa_mask);
        sigemptyset(&act_s.sa_mask);
        act_i.sa_flags = 0;
        act_s.sa_flags = 0;
        act_i.sa_handler= handler_increment;
        act_s.sa_handler= handler_stop;

        if (sigaction(SIGRTMIN, &act_i, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        if (sigaction(SIGRTMAX, &act_s, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

       // signal(SIGRTMIN, handler_increment);
       // signal(SIGRTMAX, handler_stop);

        for(int i = 0; i<N; i++){
            kill(catcher_pid, SIGRTMIN);
        }
        kill(catcher_pid, SIGRTMAX);
    }


    while(waiting_for_incoming_signals==1){
        pause();
    }

    return 0;
}
