//
// Created by werka on 4/12/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv) {
    if (argc <= 3) {
        printf("You specified too few arguments!");
        return 1;
    }

    FILE * fifo = fopen(argv[1], "r");
    FILE * fp = fopen(argv[2], "w+");
    int N = atoi(argv[3]);

    char * buf = (char *)calloc(N+13, sizeof(char));
    while(fread(buf, sizeof(char), N+13, fifo)>0){
        fwrite(buf, sizeof(char), N+13, fp);
        memset(buf,' ',strlen(buf));
    }

    fclose(fp);
    fclose(fifo);
    free(buf);
    return 0;
}
