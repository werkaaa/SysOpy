//
// Created by werka on 3/23/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

int get_k_number(char * line, int k){
    char * number;

    number = strtok(line, ",");

    for(int i=0; i<k; i++){
        if(number==NULL){
            printf("Line length exceeded!");
            return 999;
        }
        number = strtok(NULL, ",");
    }
    int number_ = atoi(number);
    return number_;
}

int * get_column(char * path, int k){
    FILE * fp = fopen(path, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    char *line = NULL;
    size_t len = 0;

    int column_size = 0;

    while(getline(&line, &len, fp) != -1) {
        column_size++;
    }
    rewind(fp);

    int * column = (int *)calloc(column_size+1, sizeof(int));

    //store size of the column on its first position
    column[0] = column_size;
    int i = 1;
    while(getline(&line, &len, fp) != -1) {
        column[i] = get_k_number(line, k);
        i++;
    }

    fclose(fp);
    free(line);

    return column;
}

int * get_row(char * line, int line_len){
    int * row = (int *)calloc(line_len+1, sizeof(int)); //a little bit too many
    char * number = strtok(line, ",");
    int row_size = 0;

    while(number!=NULL){
        row_size++;
        row[row_size] = atoi(number);
        number = strtok(NULL, ",");
    }

    free(number);

    row[0] = row_size;
    return row;
}

int multiply_row_column(int * row, int * column){
    if(row[0]!=column[0]){
        printf("Size of row or column is invalid! %dx%d\n", row[0], column[0]);
        return 0;
    }
    int n = row[0];

    int product = 0;
    for(int i = 1; i<=n; i++){
        product+=row[i]*column[i];
    }
    return product;
}

void get_sizes(char * path_in_A, char * path_in_B, int * k, int * n, int * m){
    *k=0, *n=0, *m=0;

    FILE * f_A = fopen(path_in_A, "r");
    FILE * f_B = fopen(path_in_B, "r");

    if (f_A == NULL || f_B == NULL) {
        exit(EXIT_FAILURE);
    }
    char *line = NULL;
    size_t line_len = 0;

    while(getline(&line, &line_len, f_A) != -1) {
        (*k)++;
    }

    char * token = strtok(line, ",");
    while(token!=NULL){
        (*n)++;
        token = strtok(NULL, ",");
    }

    getline(&line, &line_len, f_B);
    token = strtok(line, ",");
    while(token!=NULL){
        (*m)++;
        token = strtok(NULL, ",");
    }

    free(line);
    free(token);
    fclose(f_A);
    fclose(f_B);
}

int prepare_file(char * path_out, int k, int n, int m){

    FILE * f_out = fopen(path_out, "w+");

    if (f_out == NULL)
        exit(EXIT_FAILURE);


    int field_size = 6+n/10+1; //maksymalnie 6-cyfrowa liczba z pojedyńczego mnożenia, każde 10 takich liczb to nowe miejsce i przecinek
    char * field = (char *)calloc(field_size, sizeof(char));


    for(int i = 0;i<field_size-1; i++){
        strcat(field, "_");
    }
    strcat(field, ",");

    char * line = (char *)calloc(m*field_size+1, sizeof(char));
    for(int j=0; j<m; j++){
        strcat(line, field);
    }
    line[m*field_size-1] = '\n';

    for(int i = 0; i<k; i++){
        fwrite(line, sizeof(char), m*field_size, f_out);
    }

    fclose(f_out);
    free(line);
    free(field);

    return field_size;
}

char * get_pid_path(char * path, pid_t pid_num){
    char * out_path = calloc(strlen(path)+12, sizeof(char));
    strcpy(out_path, "tmp/");
    strcat(out_path, path);
    char * field = calloc(8, sizeof(char));
    snprintf(field, 8, "%d", pid_num);
    strcat(out_path, field);
    free(field);
    return out_path;
}

void multiply_unit(char * path_in_A, char * path_in_B, char * path_out, int c, int delta, int field_size, int k, int m, char * mode) {

    FILE *f_A = fopen(path_in_A, "r");
    int *column, *row;

    if (f_A == NULL)
        exit(EXIT_FAILURE);
    char *line = NULL;
    size_t line_len = 0;
    int out_line_len = 0;
    char *field = (char *) calloc(field_size, sizeof(char));
    int column_index = 0;

    int *column_out = (int *)calloc(k, sizeof(int));
    while (getline(&line, &line_len, f_A) != -1) {
        row = get_row(line, line_len);
        column = get_column(path_in_B, c);
        column_out[column_index] = multiply_row_column(row, column);
        column_index++;
    }
    free(column);
    free(row);

    if(strcmp(mode, "one_file")==0){
        FILE *f_out = fopen(path_out, "r+");
        if (f_out == NULL)
            exit(EXIT_FAILURE);
        flock(fileno(f_out), LOCK_EX);
        out_line_len = field_size*m;
        for(int i = 0; i<k; i++){
            snprintf(field, field_size, "%d", column_out[i]);
            fseek(f_out, i*out_line_len+c*field_size, 0);
            fwrite(field, sizeof(char), strlen(field), f_out);
        }
        flock(fileno(f_out), LOCK_UN);
        fclose(f_out);
    }
    else if(strcmp(mode, "more_files")==0){
        path_out = get_pid_path(path_out, getpid());
        FILE *f_out = fopen(path_out, "r+");
        if (f_out == NULL)
            exit(EXIT_FAILURE);

        out_line_len = field_size*m;
        for(int i = 0; i<k; i++){
            snprintf(field, field_size, "%d", column_out[i]);
            fseek(f_out, i*out_line_len+(c-delta)*field_size, 0);
            fwrite(field, sizeof(char), strlen(field), f_out);
        }
        fclose(f_out);
    }

    free(line);
    free(field);
    free(column_out);

    fclose(f_A);
}

void delete_redundancy(char * path){

    FILE * fp = fopen(path, "r+");

    if (fp == NULL)
        exit(EXIT_FAILURE);
    char *line = NULL;
    size_t line_len = 0;
    int line_num = 0;
    int str_size;

    while(getline(&line, &line_len, fp) != -1) {
        str_size = strlen(line);
        int j = 0;
        int i = 0;
        for(; i<str_size-1; i++){
            if(line[i]!='_'){
                line[j]=line[i];
                j++;
            }
        }
        for(i=j; i<str_size-1; i++){
            line[i]=' ';
        }

        fseek(fp, line_num*str_size, 0);
        fwrite(line, sizeof(char), strlen(line), fp);
        line_num++;
    }
    free(line);
    fclose(fp);
}

void delete_spaces(char * path){

    FILE * fp = fopen(path, "r+");

    if (fp == NULL)
        exit(EXIT_FAILURE);
    char *line = NULL;
    size_t line_len = 0;
    int line_num = 0;
    int str_size;

    while(getline(&line, &line_len, fp) != -1) {
        str_size = strlen(line);
        int j = 0;
        int i = 0;
        for(; i<str_size-1; i++){
            if(line[i]=='1' || line[i]=='2' ||line[i]=='3' ||line[i]=='4' || line[i]=='5' ||line[i]=='6' || line[i]=='7' ||line[i]=='8' ||line[i]=='9' || line[i]=='0' || line[i]==',' || line[i]=='-'){
                line[j]=line[i];
                j++;
            }
        }
        for(i=j; i<str_size-1; i++){
            line[i]=' ';
        }

        fseek(fp, line_num*str_size, 0);
        fwrite(line, sizeof(char), strlen(line), fp);
        line_num++;
    }
    free(line);
    fclose(fp);
}

int min(int a, int b){
    if(a<b)
        return a;
    return b;
}

int multiply_matrices(char ** A, char ** B, char ** C, int pairs_number, int i, int N, time_t start_time, time_t time_limit, char * mode){
    int multiplications = 0;
    int k, n, m, start, end, col_num, field_size, r, delta;
    for(int p = 0; p<pairs_number; p++){
        get_sizes(A[p], B[p], &k, &n, &m);
        delta=0;
        if(i>=m){
            start=end=-1;
            continue;
        }
        else {
            r = m % N;
            if (i < r) {
                col_num = m / N + 1;
                start = i * col_num;
            } else {
                col_num = m / N;
                start = i * col_num + r;
            }

            end = start +col_num -1;
        }

        field_size = 6+n/10+1;

        if(strcmp(mode, "more_files")==0){
            char * path_out = get_pid_path(C[p], getpid());
            m = end-start+1;
            prepare_file(path_out, k, n, m);
            free(path_out);
            delta = start;
        }


        for(int c = start; c<=end; c++){

            multiply_unit(A[p], B[p], C[p], c, delta, field_size, k, m, mode);


            if((int)((clock()-start_time)/CLOCKS_PER_SEC)>=time_limit){
                return (multiplications);
            }

        }
        multiplications++;
    }
    return multiplications;
}


int main(int argc, char ** argv){

    if(argc<7){
        printf("You specified too few arguments!");
        return 1;
    }

    char * list = argv[1];
    int N = atoi(argv[2]);
    time_t t = atoi(argv[3]);
    char * mode = argv[4];
    int cpu_time_limit = atoi(argv[5]);
    long virtual_memory_limit = atoi(argv[6]);

    if(strcmp(mode, "one_file")!=0 && strcmp(mode, "more_files")!=0){
        printf("argument %s not known!", mode);
    }


    FILE * fp = fopen(list, "r");
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
    free(line);

    if(strcmp(mode, "one_file")==0) {
        int k, n, m;
        for (int p = 0; p < pairs_number; p++) {

            get_sizes(A[p], B[p], &k, &n, &m);
            prepare_file(C[p], k, n, m);
        }
    }

    pid_t * children = (pid_t *)calloc(N, sizeof(pid_t));
    for(int i = 0; i<N; i++){
        pid_t child_pid = fork();
        if(child_pid!=0){
            children[i] = child_pid;
        }
        else{
            //set max CPU time
            struct rlimit * time_l = (struct rlimit *)calloc(1, sizeof(struct rlimit));
            time_l->rlim_cur = cpu_time_limit;
            time_l->rlim_max = cpu_time_limit;
            setrlimit(RLIMIT_CPU, time_l);
            free(time_l);

            //set max virtual memory
            struct rlimit * memory_l = (struct rlimit *)calloc(1, sizeof(struct rlimit));
            memory_l->rlim_cur = virtual_memory_limit * (1 << 20);
            memory_l->rlim_max = virtual_memory_limit * (1<<20);
            setrlimit(RLIMIT_AS, memory_l);
            free(memory_l);

            int multiplications = multiply_matrices(A, B, C, pairs_number, i, N, clock(), t, mode);
            exit(multiplications);
        }
    }
    struct rusage * before = (struct rusage *)calloc(1, sizeof(struct rusage));
    struct rusage * after = (struct rusage *)calloc(1, sizeof(struct rusage));
    //struct rusage * from_wait = (struct rusage *)calloc(1, sizeof(struct rusage));
    getrusage(RUSAGE_CHILDREN, after);

    struct timeval diff_stime;
    struct timeval diff_utime;

    for(int i = 0; i < N; i++) {
        memcpy(before, after, sizeof(struct rusage));
        int status;

        //wait4(children[i], &status, 0, from_wait); this solution is also possible

        waitpid(children[i], &status, 0);
        getrusage(RUSAGE_CHILDREN, after); //RUSAGE_CHILDREN - sum of successors' times

        timersub(&after->ru_utime, &before->ru_utime, &diff_utime);
        timersub(&after->ru_stime, &before->ru_stime, &diff_stime);

        printf("Process %d done %d multiplying operations\n", children[i], WEXITSTATUS(status));
        //printf("~User CPU time: %ld seconds %ld microseconds\n~System CPU time: %ld seconds %ld microseconds\n",
        // from_wait->ru_utime.tv_sec, from_wait->ru_utime.tv_usec, from_wait->ru_stime.tv_sec, from_wait->ru_stime.tv_usec);
        printf("~User CPU time: %ld seconds %ld microseconds\n~System CPU time: %ld seconds %ld microseconds\n",
                diff_utime.tv_sec, diff_utime.tv_usec, diff_stime.tv_sec, diff_stime.tv_usec);
    }
    free(before);
    free(after);


    if(strcmp(mode, "one_file")==0) {
        for (int p = 0; p < pairs_number; p++) {
            delete_redundancy(C[p]);
        }
    }
    else if(strcmp(mode, "more_files")==0){

        for(int i = 0; i < pairs_number; i++) {
            pid_t child_pid = fork();

            if(child_pid == 0) {
                char** args = (char**)calloc(N + 4, sizeof(char*));
                args[0] = (char*)calloc(6, sizeof(char));
                strcpy(args[0], "paste");
                args[1] = (char*)calloc(6, sizeof(char));
                strcpy(args[1], "-d");
                args[2] = (char*)calloc(6, sizeof(char));
                strcpy(args[2], ",");

                int k, m, n;
                get_sizes(A[i], B[i], &k, &n, &m);


                for(int j = 0; j<min(m, N); j++){
                    args[j+3] = get_pid_path(C[i], children[j]);
                    delete_redundancy(args[j+3]);
                }

                int fd = open(C[i], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                dup2(fd, 1);
                close(fd);
                execvp(args[0], args);

            }
            else {
                waitpid(child_pid, NULL, 0);
                delete_spaces(C[i]);
            }
        }

    }

    free(children);

    return 0;

}