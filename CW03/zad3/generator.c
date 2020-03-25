//
// Created by werka on 3/24/20.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int random_in(int a, int b){
    return (rand()%(b-a))+a;
}

void generate_matrix(int k, int n, char * path){
    char * line = calloc(n*6, sizeof(char));

    char * r_ = calloc(5, sizeof(char));
    int r;
    FILE * f_A = fopen(path, "w+");
    if(f_A==NULL) {
        printf("Error while reading a file!");
        return;
    }
    for(int j = 0; j<k; j++){
        strcpy(line, "");
        for(int z = 0; z < n; z++){
            r = random_in(-100, 100);
            snprintf(r_, 5, "%d", r);
            strcat(line, r_);
            strcat(line, ",");
        }
        line[strlen(line)-1] = '\n';
        fwrite(line, sizeof(char), strlen(line), f_A);
    }
    fclose(f_A);
    free(r_);
    free(line);
}

void generate_matrices(int min, int max, int i){

    int k = random_in(min, max);
    int n = random_in(min, max);
    int m = random_in(min, max);

    char * i_ = calloc(8, sizeof(char));
    snprintf(i_, 8, "%d", i);
    char * path_A = calloc(16, sizeof(char));
    char * path_B = calloc(16, sizeof(char));
    char * path_out = calloc(16, sizeof(char));
    strcpy(path_A, "files/A");
    strcpy(path_B, "files/B");
    strcpy(path_out, "files/out");
    strcat(path_A, i_);
    strcat(path_B, i_);
    strcat(path_out, i_);

    generate_matrix(k, n, path_A);
    generate_matrix(n, m, path_B);

    char * lines_path = calloc(12, sizeof(char));
    strcpy(lines_path, "files/list");

    char * line = calloc(strlen(path_A)+strlen(path_B)+strlen(path_out)+4, sizeof(char));
    strcpy(line, path_A);
    strcat(line, " ");
    strcat(line, path_B);
    strcat(line, " ");
    strcat(line, path_out);
    strcat(line, "\n");

    FILE * fp = fopen(lines_path, "a+");
    fwrite(line, sizeof(char), strlen(line), fp);
    fclose(fp);

    free(line);
    free(lines_path);
    free(path_A);
    free(path_B);
    free(path_out);
    free(i_);

}


int main(int argc, char ** argv) {
    srand(time(NULL));
    if (argc < 3) {
        printf("You specified too few arguments!");
        return 1;
    }

    int min = atoi(argv[1]);
    int max = atoi(argv[2]);
    int N = atoi(argv[3]);

    for(int i = 0; i<N; i++){
        generate_matrices(min, max, i);
    }
    return 0;
}

