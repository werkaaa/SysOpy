//
// Created by werka on 4/12/20.
//
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>

int main(){
    char * myfifo = "my_fifo";

    if(mkfifo(myfifo, 0666)<0){
        printf("Error while creating fifo!");
        return 1;
    }
    char N[2] = "10";

    if(fork()==0){
        execl("./consumer", "./consumer", myfifo, "./files/output.txt", N, NULL);
    }

    char * file_path = (char*)calloc(32, sizeof(char));

    for(int i = 0; i<5; i++){
        if(fork()==0) {
            sprintf(file_path, "./files/file%d.txt", i + 1);
            execl("./producer", "./producer", myfifo, file_path, N, NULL);
        }
    }

    for(int i = 0; i<6; i++){
        waitpid(-1, NULL, 0);
    }

    unlink(myfifo);
    free(file_path);
    
    return 0;
}
