//
// Created by werka on 3/8/20.
//

#include "diff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>

clock_t start_time, end_time;
struct tms *start_cpu;
struct tms *end_cpu;


void start_timer(){
    start_time = times(start_cpu);
}

void stop_timer(FILE * file){
    end_time = times(end_cpu);
    int ticks = sysconf(_SC_CLK_TCK);

    double real_time = (double)(end_time-start_time)/ticks;
    double system_time = (double)(end_cpu->tms_stime-start_cpu->tms_stime)/ticks;
    double user_time = (double)(end_cpu->tms_utime-start_cpu->tms_utime)/ticks;

    fprintf(file, "------\nReal time: %f\nSystem time: %f\nUser time: %f\n------\n", real_time, system_time, user_time);
    printf("------\nReal time: %f\nSystem time: %f\nUser time: %f\n------\n", real_time, system_time, user_time);

}


int main(int argc, char **argv) {
    start_cpu = calloc(1, sizeof(struct tms));
    end_cpu = calloc(1, sizeof(struct tms));

    main_array * array_of_blocks = NULL;
    int size;

    FILE * time_report;


    if(argc<=1){
        printf("You haven't entered the arguments\n");
        return 1;
    }

    time_report = fopen("results3b.txt", "a");

    for(int i = 1; i<argc; i++){
        if(strcmp(argv[i], "create_table")==0){
            if(i+1 == argc){
                printf("You haven't specified main array size\n");
                return 1;
            }
            size = atoi(argv[i+1]);
            if(size<=0){
                printf("Array size should be a positive integer\n");
                return 1;
            }

            array_of_blocks = create_main_array(size);
            i += 1;
        }
        else if(strcmp(argv[i], "compare_pairs")==0) {

            if(i+1>=argc){
                printf("Number of files not specified\n");
            }
            i = i+1;

            int files_number = atoi(argv[i]);

            if(i+files_number>=argc){
                printf("Too few file arguments\n");
            }

            pair * pairs_to_compare = create_pair_sequence(&argv[i+1], files_number);
            print_pair_sequence(pairs_to_compare, files_number);


            for(int j = 0; j<files_number; j++) {
                char * file_name = compare_pair(&pairs_to_compare[j]);
                save_block_to_main_array(array_of_blocks, file_name);
            }

            //print_main_array(array_of_blocks);

            i = i+files_number;
        }
        else if(strcmp(argv[i], "remove_block")==0) {
            if(i+1 == argc){
                printf("You haven't specified index of the block to be removed\n");
                return 1;
            }
            int block_id = atoi(argv[i+1]);
            delete_block(array_of_blocks, block_id);

            //print_main_array(array_of_blocks);

            i += 1;
        }
        else if(strcmp(argv[i], "remove_operation")==0) {
            if(i+2 >= argc){
                printf("You haven't specified enough arguments for operation removal\n");
                return 1;
            }
            int block_to_remove_from = atoi(argv[i+1]);
            int operation_id = atoi(argv[i+2]);

            delete_operation(array_of_blocks->blocks[block_to_remove_from], operation_id);

            //print_main_array(array_of_blocks);

            i += 2;
        }
        else if(strcmp(argv[i], "start_timer")==0){

            fprintf(time_report, "%s\n", argv[i+1]);
            start_timer();
            i += 1;
        }
        else if(strcmp(argv[i], "stop_timer")==0){
            stop_timer(time_report);

        }
        else {
            printf("You entered not known argument\n");
        }
    }

    fclose(time_report);
    free_main_array(array_of_blocks);

    return 0;
}

