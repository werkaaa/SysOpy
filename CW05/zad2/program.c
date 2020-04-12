//
// Created by werka on 4/12/20.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int main(int argc, char ** argv){
    if(argc<=1){
        printf("You specified too few arguments!");
        return 1;
    }

    char * command = (char*)calloc(6 + strlen(argv[1]), sizeof(char));
    strcpy(command, "sort ");
    strcat(command, argv[1]);
    printf("Command: %s\n", command);

    FILE* stream = popen(command, "r");

    if (stream == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, stream)) != -1) {
        fwrite(line, nread, 1, stdout);
    }

    free(line);

    pclose(stream);


    return 0;

}