//
// Created by werka on 5/5/20.
//
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>

#define UPPER_BOUND 256
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int ** image;
int ** threads_storage;
int width, height, threads_number;
char * mode;

void error(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
}

void read_image(char * file_path){
    FILE *f = fopen(file_path, "r");
    if(f==NULL) error("Problem with file opening!\n");
    char *line_buf = NULL;
    size_t line_buf_size = 0;

    if(getline(&line_buf, &line_buf_size, f)==-1) error("Problem with reading file!\n");
    if(getline(&line_buf, &line_buf_size, f)==-1) error("Problem with reading file!\n");
    width = atoi(strtok(line_buf, " "));
    height = atoi(strtok(NULL, " "));

    image = (int **)calloc(height, sizeof(int *));
    for(int i = 0; i<height; i++)
        image[i] = (int *)calloc(width, sizeof(int));

    if(getline(&line_buf, &line_buf_size, f)==-1) error("Problem with reading file!\n");

    int i=0;
    char *token;
    while(i<width*height) {
        if (getline(&line_buf, &line_buf_size, f) == -1) error("Problem with reading file!\n");
        token = strtok(line_buf, " ");
        while(token!=NULL){
            image[i/width][i%width] = atoi(token);
            i++;
            token = strtok(NULL, " ");
        }
    }

    fclose(f);
}

long int timespec_diff(struct timespec *start, struct timespec *stop)
{
    struct timespec result;
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result.tv_sec = stop->tv_sec - start->tv_sec - 1;
        result.tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result.tv_sec = stop->tv_sec - start->tv_sec;
        result.tv_nsec = stop->tv_nsec - start->tv_nsec;
    }

    return 1000000*result.tv_sec + (long int)round(result.tv_nsec/1000.0);
}

void* thread_part(void* index_) {
    struct timespec start;
    struct timespec end;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) error("Time problem!");

    int index = *((int *) index_);

    if(strcmp(mode, "sign")==0){
        for(int i = 0; i<height; i++){
            for(int j = 0; j<width; j++){
                if(image[i][j]%threads_number==index)
                    threads_storage[index][image[i][j]]++;
            }
        }
    }
    else if(strcmp(mode, "block")==0){
        int w = ceil((double)width/threads_number);
        for(int i = index*w; i<MIN(width, (index+1)*w); i++){
            for(int j = 0; j<height; j++) {
                threads_storage[index][image[j][i]]++;
            }
        }
    }
    else if(strcmp(mode, "interleaved")==0){
        for(int i = index; i<width; i=i+threads_number){
            for(int j = 0; j<height; j++) {
                threads_storage[index][image[j][i]]++;
            }
        }
    }
    else{
        error("Wrong mode name!");
    }

    if (clock_gettime(CLOCK_REALTIME, &end) == -1) error("Time problem!");

    long int* delta_time = (long int*) malloc(sizeof(long int));

    *delta_time = timespec_diff(&start, &end);
    pthread_exit((void *) delta_time);
}

void save_histogram(char * file_path){
    FILE *f = fopen(file_path, "w");
    if(f==NULL) error("Problem with file opening!");
    int counted;
    for(int i = 0; i<UPPER_BOUND; i++){
        counted=0;
        for(int t = 0; t<threads_number; t++)
            counted += threads_storage[t][i];
        fprintf(f, "%d %d\n", i, counted);
    }
    fclose(f);
}

int main(int argc, char ** argv){
    if(argc<=4){
        printf("You specified too few arguments!");
        return 1;
    }

    threads_number = atoi(argv[1]);
    mode = argv[2];
    read_image(argv[3]);

    threads_storage = (int **) calloc(threads_number, sizeof(int *));

    struct timespec start;
    struct timespec end;

    for(int i = 0; i<threads_number; i++) {
        threads_storage[i] = (int *) calloc(UPPER_BOUND, sizeof(int));
        for(int j = 0; j<UPPER_BOUND; j++)
            threads_storage[i][j] = 0;
    }

    pthread_t* threads = (pthread_t *)calloc(threads_number, sizeof(pthread_t));
    long int * times = (long int *)calloc(threads_number, sizeof(long int));

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) error("Time problem!");

    for(int i = 0; i<threads_number; i++){
        int* i_ = (int*) malloc(sizeof(int));
        *i_ = i;
        if(pthread_create(&threads[i], NULL, thread_part, (void *)i_)!=0) error("Problem with thread creation!\n");
    }

    for(int i = 0; i<threads_number; i++) {
        times[i] = 0;
        void* return_val;
        if(pthread_join(threads[i], &return_val)!=0) error("Problem with thread!\n");
        times[i] =  *((long int*) return_val);
    }

    if (clock_gettime(CLOCK_REALTIME, &end) == -1) error("Time problem!");

    long int delta_time = timespec_diff(&start, &end);

    printf("Mode: %s, Threads: %d\n", mode, threads_number);
    for(int i = 0; i<threads_number; i++) {
        printf("\tThread: %d, Thread ID: %ld, Time: %ldμs\n", i, threads[i], times[i]);
    }

    printf("\n\tTime total: %ldμs\n\n", delta_time);
    save_histogram(argv[4]);

    for(int i = 0; i<threads_number; i++)
        free(threads_storage[i]);

    free(threads_storage);
    free(threads);
    free(times);

    for(int i = 0; i<height; i++) free(image[i]);
    free(image);

    return 0;
}
