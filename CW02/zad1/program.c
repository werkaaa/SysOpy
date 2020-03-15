//
// Created by werka on 3/11/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/times.h>

void generate_record(char * buf, int record_length){

    char letter;
    for(int j = 0; j<record_length; j++){
        letter = (char) (rand()%26+65);
        buf[j] = letter;
    }
    buf[record_length] = '\n';
}

char * get_path(char * name){
    char * path = calloc(6 + sizeof(name), sizeof(char));
    strcat(path, "files/");
    strcat(path, name);
    return path;
}

void generate_lib(char * name, int records_number, int record_length){
    char * path = calloc(6 + sizeof(name), sizeof(char));
    strcat(path, "files/");
    strcat(path, name);
    printf("%s", path);
    FILE * fp = fopen(path, "w");

    char * buf = calloc(record_length+1, sizeof(char));
    srand(time(0));

    for(int i = 0; i<records_number; i++){
        generate_record(buf, record_length);
        fwrite(buf, sizeof(char), record_length+1, fp);
    }
    free(buf);
    fclose(fp);
}


void generate_sys(char * name, int records_number, int record_length){
    char * path = get_path(name);
    printf("%s", path);
    int fd = open(path, O_WRONLY|O_CREAT); //create write-only file //this doesn't work

    char * buf = calloc(record_length+1, sizeof(char));
    srand(time(0));

    for(int i = 0; i<records_number; i++){
        generate_record(buf, record_length);
        write(fd, buf, record_length+1);
    }
    free(buf);
    close(fd);
}

void swap_lib(FILE * fp, int i, int j, int record_length, char * pointer_0, char * pointer_1){

    fseek(fp, i*(record_length+1), SEEK_SET);
    fread(pointer_0, sizeof(char), record_length, fp);
    fseek(fp, j*(record_length+1), SEEK_SET);
    fread(pointer_1, sizeof(char), record_length, fp);

    fseek(fp, i*(record_length+1), SEEK_SET);
    fwrite(pointer_1, sizeof(char), record_length, fp);
    fseek(fp, j*(record_length+1), SEEK_SET);
    fwrite(pointer_0, sizeof(char), record_length, fp);
}

int partition_lib(FILE * fp, int low, int high, int record_length){

    char * pivot = calloc(record_length, sizeof(char));
    char * pointer = calloc(record_length, sizeof(char));

    int i = high+1;
    for(int j = high; j>=low; j--){

        fseek(fp, low*(record_length+1), SEEK_SET);
        fread(pivot, sizeof(char), record_length, fp);

        fseek(fp, j*(record_length+1), SEEK_SET);
        fread(pointer, sizeof(char), record_length, fp);

        if(strcmp(pivot, pointer)<0){
            i--;
            swap_lib(fp, i, j, record_length, pivot, pointer);
        }
    }
    i--;
    swap_lib(fp, i, low, record_length, pivot, pointer);

    free(pivot);
    free(pointer);
    //printf("%d", i);
    return i;
}

void quick_sort_lib(FILE * fp, int low, int high, int record_length){
    if(low<high){
        int pivot = partition_lib(fp, low, high, record_length);
        quick_sort_lib(fp, low, pivot-1, record_length);
        quick_sort_lib(fp, pivot+1, high, record_length);
    }
}

void sort_lib(char * name, int records_number, int record_length){
    char * path = get_path(name);
    FILE * fp = fopen(path, "r+");
    quick_sort_lib(fp, 0, records_number-1, record_length);
    fclose(fp);
}

void swap_sys(int fd, int i, int j, int record_length, char * pointer_0, char * pointer_1){

    lseek(fd, i*(record_length+1), 0);
    read(fd, pointer_0, record_length);
    lseek(fd, j*(record_length+1), 0);
    read(fd, pointer_1, record_length);

    lseek(fd, i*(record_length+1), 0);
    write(fd, pointer_1, record_length);
    lseek(fd, j*(record_length+1), 0);
    write(fd, pointer_0, record_length);
}

int partition_sys(int fd, int low, int high, int record_length){

    char * pivot = calloc(record_length, sizeof(char));
    char * pointer = calloc(record_length, sizeof(char));

    int i = high+1;
    for(int j = high; j>=low; j--){

        lseek(fd, low*(record_length+1), 0);
        read(fd, pivot, record_length);

        lseek(fd, j*(record_length+1), 0);
        read(fd, pointer, record_length);

        if(strcmp(pivot, pointer)<0){
            i--;
            swap_sys(fd, i, j, record_length, pivot, pointer);
        }
    }
    i--;
    swap_sys(fd, i, low, record_length, pivot, pointer);

    free(pivot);
    free(pointer);

    return i;
}

void quick_sort_sys(int fd, int low, int high, int record_length){
    if(low<high){
        int pivot = partition_sys(fd, low, high, record_length);
        quick_sort_sys(fd, low, pivot-1, record_length);
        quick_sort_sys(fd, pivot+1, high, record_length);
    }
}

void sort_sys(char * name, int records_number, int record_length){
    char * path = get_path(name);
    int fd = open(path, O_RDWR);
    quick_sort_sys(fd, 0, records_number-1, record_length);
    close(fd);
}
void copy_sys(char * name_1, char * name_2, int records_number, int record_length){
    int fd_1 = open(get_path(name_1), O_RDONLY);
    int fd_2 = open(get_path(name_2), O_WRONLY);
    char * record = calloc(record_length, sizeof(char));

    for(int i = 0; i<records_number; i++){
        read(fd_1, record, record_length+1);
        write(fd_2, record, record_length+1);
    }
    close(fd_1);
    close(fd_2);
    free(record);
}

void copy_lib(char * name_1, char * name_2, int records_number, int record_length){
    FILE * fp_1 = fopen(get_path(name_1), "r");
    FILE * fp_2 = fopen(get_path(name_2), "w");
    char * record = calloc(record_length, sizeof(char));

    for(int i = 0; i<records_number; i++){
        fread(record, sizeof(char), record_length+1, fp_1);
        fwrite(record, sizeof(char), record_length+1, fp_2);
    }
    fclose(fp_1);
    fclose(fp_2);
    free(record);
}

clock_t start_time, end_time;
struct tms *start_cpu;
struct tms *end_cpu;


void start_timer(){
    start_time = times(start_cpu);
}

void stop_timer(FILE * file){
    end_time = times(end_cpu);
    int ticks = sysconf(_SC_CLK_TCK);

    double system_time = (double)(end_cpu->tms_stime-start_cpu->tms_stime)/ticks;
    double user_time = (double)(end_cpu->tms_utime-start_cpu->tms_utime)/ticks;

    fprintf(file, "------\nSystem time: %f\nUser time: %f\n------\n", system_time, user_time);
    printf("------\nSystem time: %f\nUser time: %f\n------\n", system_time, user_time);

}

int main(int argc, char **argv){
    start_cpu = calloc(1, sizeof(struct tms));
    end_cpu = calloc(1, sizeof(struct tms));

    if(argc<=1){
        printf("You haven't entered the arguments\n");
        return 1;
    }

    FILE * time_report = fopen("wyniki.txt", "a");

    for(int i = 1; i<argc; i++){
        if(strcmp(argv[i], "generate")==0){
            char * file_name = argv[i+1];
            int records_number = atoi(argv[i+2]);
            int record_length = atoi(argv[i+3]);
            generate_lib(file_name, records_number, record_length);
            i += 3;
        }
        else if(strcmp(argv[i], "sort")==0) {
            char * file_name = argv[i+1];
            int records_number = atoi(argv[i+2]);
            int record_length = atoi(argv[i+3]);
            char * version = argv[i+4];
                if(strcmp(version, "sys")==0){
                    sort_sys(file_name, records_number, record_length);
                }
                else if(strcmp(version, "lib")==0){
                    sort_lib(file_name, records_number, record_length);
                }
                else{
                    printf("There was some problem with sort function call!");
                }
            i += 4;
        }
        else if(strcmp(argv[i], "copy")==0) {
            char * file_1_name = argv[i+1];
            char * file_2_name = argv[i+2];
            int records_number = atoi(argv[i+3]);
            int record_length = atoi(argv[i+4]);
            char * version = argv[i+5];
            if(strcmp(version, "sys")==0){
                copy_sys(file_1_name, file_2_name, records_number, record_length);
            }
            else if(strcmp(version, "lib")==0){
                copy_lib(file_1_name, file_2_name, records_number, record_length);
            }
            else{
                printf("There was some problem with copy function call!");
            }
            i += 5;
        }
        else if(strcmp(argv[i], "start_timer")==0){
            //fprintf(time_report, "%s\n", argv[i+1]);
            start_timer();
            //i += 1;
        }
        else if(strcmp(argv[i], "stop_timer")==0){
            stop_timer(time_report);

        }
        else {
            printf("You entered not known argument %s\n", argv[i]);
        }
    }

    fclose(time_report);

    return 0;
}