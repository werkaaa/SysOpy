//
// Created by werka on 3/30/20.
//
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

void handler_receive_value(int sig, siginfo_t *info, void *ucontext) {
    printf("\nReceived signal \"%s\" ", strsignal(sig));
    printf("from process %d ", info->si_pid);
    printf("with message %d.\n", info->si_int);
}

void handler_segfault(int sig, siginfo_t *info, void *ucontext) {
    printf("\nReceived signal \"%s\". ", strsignal(sig));
    printf("Memory location which caused fault: %p.\n", info->si_addr);
    exit(0);
}

void handler_child_info(int sig, siginfo_t *info, void *ucontext) {
    printf("\nReceived signal \"%s\". ", strsignal(sig));
    printf("Child PID: %d. ", info->si_pid);
    printf("User time consumed by child: %fs. ", (double)info->si_utime/CLOCKS_PER_SEC);
    printf("System time consumed by child: %fs. ", (double)info->si_stime/CLOCKS_PER_SEC);
    printf("Child status: %d.\n", info->si_status);
}

int main(int argc, char **argv){

    if(argc<=1){
        printf("You haven't entered the arguments\n");
        return 1;
    }

    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    if(strcmp(argv[1], "send_value")==0){
        //handler pozyskuje informację o przesłanej wraz z sygnałem SIGUSR1 wartości calkowitej

        act.sa_sigaction= handler_receive_value;

        if (sigaction(SIGUSR1, &act, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        if(argc==2){
            printf("Value to send not specified!\n");
            return 1;
        }

        union sigval value;
        value.sival_int = atoi(argv[2]);
        sigqueue(getpid(), SIGUSR1, value);
    }
    else if(strcmp(argv[1], "force_segfault")==0){
        //handler pozyskuje informację o miejscu w pamięci, gdzie wystąpił Segfault

        act.sa_sigaction= handler_segfault;
        if (sigaction(SIGSEGV, &act, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        char *c = "Hello";
        c[10] = 'z';
    }
    else if(strcmp(argv[1], "get_child_info")==0){
        //handler pozyskuje informację o procesie potomnym, jego czas użytkownika, systemowy oraz status

        act.sa_sigaction= handler_child_info;
        if (sigaction(SIGCHLD, &act, NULL) < 0) {
            perror ("sigaction");
            return 1;
        }

        pid_t child_pid = fork();
        if(child_pid<0){
            perror("fork");
            return 1;
        }
        else if(child_pid==0) {
            int d;
            for(int i=0; i<1000000000; i++){
                d = 2/2;
            }
            exit(d);
        }
        else {
            waitpid(child_pid, NULL, 0);
        }
    }
    else {
        printf("You specified not known argument! %s\n", argv[1]);
    }
}
