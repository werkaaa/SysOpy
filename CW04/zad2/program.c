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

    if(argc<=2){
        printf("You haven't entered the arguments\n");
        return 1;
    }

    char * test = argv[2];


    if(strcmp(argv[1], "ignore")==0){
            signal(SIGUSR1, SIG_IGN);
            raise(SIGUSR1);


            struct sigaction oact;

            if (sigaction(SIGUSR1, NULL, &oact) < 0) {
                perror("sigaction");
                return 1;
            }
            if (oact.sa_handler == SIG_IGN)
                printf("Parent ignores signal\n");
            else
                printf("Parent doesn't ignore signal\n");

            if(strcmp(test, "fork")==0){
            pid_t child_pid = fork();

            if(child_pid<0){
                perror("fork");
                return 1;
            }
            else if(child_pid==0) {
                raise(SIGUSR1);

                struct sigaction oact;

                if (sigaction(SIGUSR1, NULL, &oact) < 0) {
                    perror("sigaction");
                    return 1;
                }
                if (oact.sa_handler == SIG_IGN)
                    printf("Child ignores signal\n");
                else
                    printf("Child doesn't ignore signal\n");
            }
            else {
                waitpid(child_pid, NULL, 0);
            }

            }
            else if(strcmp(test, "exec")==0){
                execl("./process_function", "./process_function", "ignore", NULL);
            }
            else{
                printf("You entered not known argument %s\n", argv[2]);
            }


        }
        else if(strcmp(argv[1], "handler")==0) {
            signal(SIGUSR1, handle_sigusr1);
            raise(SIGUSR1);

           if(strcmp(test, "fork")==0) {
               pid_t child_pid = fork();
               if (child_pid < 0) {
                   perror("fork");
                   return 1;
               } else if (child_pid == 0) {
                   raise(SIGUSR1);
               } else {
                   waitpid(child_pid, NULL, 0);
               }
           }
           else{
               printf("You entered not known argument %s\n", argv[2]);
           }
        }
        else if(strcmp(argv[1], "mask")==0) {

            sigset_t newmask;
            sigset_t mask;
            sigemptyset(&newmask);
            sigaddset(&newmask, SIGUSR1);

            if (sigprocmask(SIG_BLOCK, &newmask, NULL) < 0)
                perror("Signal wasn't blocked!");

            raise(SIGUSR1);

            sigpending(&mask);

            if (sigismember(&mask, SIGUSR1))
                printf("Parent blocks signal\n");
            else
                printf("Parent doesn't block signal\n");

            if(strcmp(test, "fork")==0) {
                pid_t child_pid = fork();
                if (child_pid < 0) {
                    perror("fork");
                    return 1;
                } else if (child_pid == 0) {
                    raise(SIGUSR1);

                    sigpending(&mask);

                    if (sigismember(&mask, SIGUSR1))
                        printf("Child blocs signal\n");
                    else
                        printf("Child doesn't block signal\n");
                } else {
                    waitpid(child_pid, NULL, 0);
                }
            }
            else if(strcmp(test, "exec")==0){
                execl("./process_function", "./process_function", "mask", NULL);
            }
            else{
                printf("You entered not known argument %s\n", argv[2]);
            }

        }
        else if(strcmp(argv[1], "pending")==0){

            sigset_t newmask;
            sigset_t mask;
            sigemptyset(&newmask);
            sigaddset(&newmask, SIGUSR1);

            if (sigprocmask(SIG_BLOCK, &newmask, NULL) < 0)
                perror("Signal wasn't blocked!");

            raise(SIGUSR1);

            sigpending(&mask);

            if (sigismember(&mask, SIGUSR1))
                printf("Parent blocks signal\n");

            if(strcmp(test, "fork")==0) {
                pid_t child_pid = fork();
                if (child_pid < 0) {
                    perror("fork");
                    return 1;
                } else if (child_pid == 0) {

                    sigpending(&mask);

                    if (sigismember(&mask, SIGUSR1))
                        printf("Signal still pending in child process\n");
                    else
                        printf("Signal isn't pending in child process\n");
                } else {
                    waitpid(child_pid, NULL, 0);
                }
            }
            else if(strcmp(test, "exec")==0){
                execl("./process_function", "./process_function", "pending", NULL);
            }
            else{
                printf("You entered not known argument %s\n", argv[2]);
            }

        }
        else {
            printf("You entered not known argument %s\n", argv[1]);
        }


    return 0;
}
