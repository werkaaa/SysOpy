//
// Created by werka on 3/30/20.
//
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void handle_sigusr1(int sig)
{
    printf("Process %d caught signal SIGUSR1\n", getpid());
}


int main(int argc, char **argv){

    if(argc<=1){
        printf("You haven't entered the arguments\n");
        return 1;
    }

    if(strcmp(argv[1], "ignore")==0){

        raise(SIGUSR1);

        struct sigaction oact;

        if (sigaction(SIGUSR1, NULL, &oact) < 0) {
            perror("sigaction");
            return 1;
        }
        if (oact.sa_handler == SIG_IGN)
            printf("Still ignores signal\n");
        else
            printf("Still doesn't ignore signal\n");

    }
    else if(strcmp(argv[1], "mask")==0){

        raise(SIGUSR1);

        sigset_t mask;

        sigpending(&mask);

        if (sigismember(&mask, SIGUSR1))
            printf("Still blocs signal\n");
        else
            printf("Still doesn't block signal\n");
    }
    else if(strcmp(argv[1], "pending")==0){

        sigset_t mask;

        sigpending(&mask);

        if (sigismember(&mask, SIGUSR1))
            printf("Signal still pending in process\n");
        else
            printf("Signal isn't pending in process\n");
    }
    else {
        printf("You entered not known argument %s\n", argv[1]);
    }


    return 0;
}

