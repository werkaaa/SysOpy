//
// Created by werka on 4/12/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char ** argv) {
    if (argc <= 3) {
        printf("You specified too few arguments!");
        return 1;
    }

    FILE * fifo = fopen(argv[1], "w+");
    FILE * fp = fopen(argv[2], "r");
    int N = atoi(argv[3]);

    char * buf = (char *)calloc(N, sizeof(char));

    pid_t my_pid = getpid();
    char * pid = (char *)calloc(10, sizeof(char));
    sprintf(pid, "#%d#", my_pid);

    char * msg = (char *)calloc(N+strlen(pid)+3, sizeof(char));

    srand(time(NULL));

    while(fread(buf, sizeof(char), N, fp)>0){
        sleep(rand()%2+1);
        strcpy(msg, pid);
        strcat(msg, buf);
        strcat(msg, "\n");
        fwrite(msg, sizeof(char), strlen(msg), fifo);

        memset(buf,0,strlen(buf));
        memset(msg,0,strlen(msg));

    }

    fclose(fp);
    fclose(fifo);
    free(buf);
    free(msg);
    free(pid);
    return 0;
}
