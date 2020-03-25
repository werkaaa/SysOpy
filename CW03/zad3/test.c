//
// Created by werka on 3/25/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Matrix{
    int width;
    int height;
    int ** data;
};

struct Matrix * read_matrix(char * path){
   struct Matrix * M = (struct Matrix *)calloc(1, sizeof(struct Matrix));

   FILE * fp = fopen(path, "r+");
   if(fp==NULL){
       printf("Error while opening the file!");
       return NULL;
   }
   int height = 0;
   char * line = NULL;
   size_t line_len = 0;
   while(getline(&line, &line_len, fp)!=-1){
       height++;
   }

   int width = 1;
   strtok(line, ",");
   while(strtok(NULL, ",")!=NULL)
       width++;
   rewind(fp);

   M->height = height;
   M->width = width;
   M->data = (int **)calloc(height, sizeof(int*));

   for(int i = 0; i<height; i++){
        getline(&line, &line_len, fp);
        M->data[i] = calloc(width, sizeof(int));
        M->data[i][0] = atoi(strtok(line, ","));
        for(int j = 1; j<width; j++){
            M->data[i][j] = atoi(strtok(NULL, ","));
        }
   }
   fclose(fp);
   free(line);
   return M;
}

struct Matrix * multiply_matrices(struct Matrix * A, struct Matrix * B){
    if(A->width!=B->height){
        printf("Wrong dimensions for multiplication!");
        return NULL;
    }
    int N = A->width;
    int sum;
    struct Matrix * M = (struct Matrix *)calloc(1, sizeof(struct Matrix));
    M->height = A->height;
    M->width = B->width;
    M->data = (int **)calloc(M->height, sizeof(int *));
    for(int i = 0; i<M->height; i++){
        M->data[i] = (int*)calloc(M->width, sizeof(int));
        for(int j = 0; j<M->width; j++){
            sum = 0;
            for(int k = 0; k<N; k++){
                sum+=A->data[i][k]*B->data[k][j];
            }
            M->data[i][j] = sum;
        }
    }
    return M;
}

int compare_matrices(struct Matrix * A, struct Matrix * B){
    if(A->width!=B->width || A->height!=B->height){
        printf("Wrong sizes!");
        return 0;
    }
    for(int i = 0; i<A->height; i++){
        for(int j = 0; j<A->width; j++){
            if(A->data[i][j]!=B->data[i][j]) {
                printf("%d, %d, %d, %d\n", i, j, A->data[i][j], B->data[i][j]);
                return 0;
            }
        }
    }
    return 1;
}

void print_matrx(struct Matrix * A){
    for(int i = 0; i<A->height; i++){
        for(int j = 0; j<A->width; j++){
            printf("%d, ", A->data[i][j]);
        }
        printf("\n");
    }
}


int main(int argc, char ** argv) {
    if(argc<2){
        printf("Nothing to test!");
        return 1;
    }
    char * path = argv[1];
    FILE * fp = fopen(path, "r");
    if(fp==NULL){
        printf("something went wrong while reading a file!");
        return 1;
    }
    char * line = NULL;
    size_t line_len = 0;
    int pairs_number = 0;
    while(getline(&line, &line_len, fp)!=-1){
        pairs_number++;
    }
    rewind(fp);
    char ** A = calloc(pairs_number, sizeof(char *));
    char ** B = calloc(pairs_number, sizeof(char *));
    char ** C = calloc(pairs_number, sizeof(char *));
    for(int i = 0; i<pairs_number; i++){
        A[i] = calloc(100, sizeof(char));
    }

    int i =0;
    while(getline(&line, &line_len, fp)!=-1){
        char * help_line = calloc(strlen(line), sizeof(char));
        strcpy(help_line, line);

        A[i] = strtok(help_line, " ");
        B[i] = strtok(NULL, " ");
        C[i] = strtok(NULL, "\n");

        i++;
    }
    struct Matrix * a;
    struct Matrix * b;
    struct Matrix * c;
    struct Matrix * m;

    int passed = 1;

    for(int i = 0; i<pairs_number; i++){
        a = read_matrix(A[i]);
        b = read_matrix(B[i]);
        c = read_matrix(C[i]);
        m = multiply_matrices(a, b);
        //print_matrx(m);
        if(compare_matrices(c, m)==0){
            printf("Error in matrix multiplication: %d\n", i);
            passed=0;
        }
        free(a);
        free(b);
        free(c);
        free(m);
    }
    if(passed==1){
        printf("All tests passed!\n");
    }

    return 0;
}