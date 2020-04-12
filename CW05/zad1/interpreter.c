//
// Created by werka on 4/12/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
const int MAX_ARG = 5;
const int MAX_COMMAND = 5;
int ACTUAL_COMMAND_NUM;

char *** divide_line(char * line){
    ACTUAL_COMMAND_NUM = 0;
    char * token;
    char * arg;
    int i = 0;
    int j;
    char *** data = (char ***)calloc(MAX_COMMAND, sizeof(char **));

    for (token = strtok (line, "|"); token != NULL; token = strtok (token + strlen(token) + 2, "|"))
    {
        data[i] = (char **)calloc(MAX_ARG+2, sizeof(char *));
        char * buf = (char*)calloc(strlen(token), sizeof(char));
        strcpy (buf, token);
        j = 0;
        for (arg = strtok (buf, " "); arg != NULL; arg = strtok (arg + strlen(arg) + 1, " ")) {
            data[i][j] = arg;
            j++;
        }

        i++;
    }

//    printf("%s_\n", data[i-1][j-1]);
    ACTUAL_COMMAND_NUM = i;

    return data;
}



int main(int argc, char ** argv){
    if(argc<=1){
        printf("You specified too few arguments!");
        return 1;
    }

    FILE *stream;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    stream = fopen(argv[1], "r");
    if (stream == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while ((nread = getline(&line, &len, stream)) != -1) {
        if(line[strlen(line)-1]=='\n'){
            line[strlen(line)-1]='\0';
        }
        printf("Command: %s\n", line);
        char *** data = divide_line(line);
        pid_t * child_pids = (pid_t *)calloc(ACTUAL_COMMAND_NUM, sizeof(pid_t));

        int fd[2];
        pipe(fd);

        pid_t child_pid = fork();
        if (child_pid == 0) {
            dup2(fd[1], STDOUT_FILENO); //wszystko, co miało iść na standardowe wyjście pójdzie teraz do fd[1], fd[1] ma teraz podwójne oznaczenie
            close(fd[1]); //dlatego zamykamy stare, teraz tylko to, co miało iść na stdout pójdzie do fd[1]
            close(fd[0]); //zamykamy też to wejście, bo nie jest nam potrzebne
            execvp(data[0][0], data[0]);
        }

        child_pids[0] = child_pid;
        int prev_fd[2];


        for(int i = 1; i<ACTUAL_COMMAND_NUM; i++){
            prev_fd[1] = fd[1];
            prev_fd[0] = fd[0];

            pipe(fd);
            child_pid = fork();

            if (child_pid == 0) {
                dup2(fd[1], STDOUT_FILENO);
                dup2(prev_fd[0], STDIN_FILENO);
                close(fd[1]);
                close(prev_fd[1]);
                close(fd[0]);
                close(prev_fd[0]);
                execvp(data[i][0], data[i]);
            }
            close(prev_fd[0]);
            close(prev_fd[1]);
            child_pids[i] = child_pid;
        }

        for(int i = 0; i<ACTUAL_COMMAND_NUM; i++){
            waitpid(child_pids[i], NULL, 0);
        }

        char* commands_output = (char*)calloc(1000, sizeof(char));
        read(fd[0], commands_output, 1000);
        printf("Command output: %s\n", commands_output);


        close(fd[1]);
        close(fd[0]);


        for(int i = 0; i<ACTUAL_COMMAND_NUM; i++){
            free(data[i][0]); //alokacja przez strtoken wymaga takiego zwolnienia pamięci
            free(data[i]);
        }
        free(data);
        free(child_pids);

    }

    free(line);
    fclose(stream);

    return 0;

}