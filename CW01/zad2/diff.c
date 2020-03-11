//
// Created by werka on 3/4/20.
//
#include "diff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



char * compare_pair(pair * pair_to_compare) {
    char * file_name = calloc(strlen(pair_to_compare->file1) + strlen(pair_to_compare->file2) + 8, sizeof(char));
    strcat(file_name, "tmp/");
    strcat(file_name, pair_to_compare->file1);
    strcat(file_name, pair_to_compare->file2);
    strcat(file_name, ".txt");

    char * command = calloc(strlen(pair_to_compare->file1) + strlen(pair_to_compare->file2) + strlen(file_name) + 22, sizeof(char));
    strcat(command, "diff files/");
    strcat(command, pair_to_compare->file1);
    strcat(command, " files/");
    strcat(command, pair_to_compare->file2);
    strcat(command, " > ");
    strcat(command, file_name);
    system(command);
    return file_name;
}


main_array * create_main_array(int sequence_size) {
    main_array * array_of_blocks = calloc(1, sizeof(main_array));
    array_of_blocks->size = sequence_size;
    array_of_blocks->last_one = -1;
    array_of_blocks->blocks = (block **) calloc(sequence_size, sizeof(block *));


    return array_of_blocks;
}

void add_block(main_array * array_of_blocks, block * block_of_operations) {
    array_of_blocks->blocks[++array_of_blocks->last_one] = block_of_operations;
}

block * create_block(int size) {
    block * block_of_operations = calloc(1, sizeof(block));
    block_of_operations->size = size;
    block_of_operations->last_one = -1;
    block_of_operations->operations = (char**) calloc(size, sizeof(char *));

    return block_of_operations;
}


int save_block_to_main_array(main_array * array_of_blocks, char * file_name) {
    FILE *in_file  = fopen(file_name, "r");
    char* line = NULL;
    size_t line_len = 0;

    int num_of_blocks = 0;

    while (getline(&line, &line_len, in_file) != -1)
    {
        if(line[0] != '<' && line[0] != '>' && line[0] != '-') {
            num_of_blocks++;
        }
    }
    rewind(in_file);

    block * block_of_operations = create_block(num_of_blocks);

    while (getline(&line, &line_len, in_file) != -1)
    {
        if(line[0] != '<' && line[0] != '>' && line[0] != '-') {

            block_of_operations->operations[++block_of_operations->last_one] = calloc(line_len, sizeof(char));
        }
        else{
            char * tmp = block_of_operations->operations[block_of_operations->last_one];
            block_of_operations->operations[block_of_operations->last_one] = realloc(tmp, strlen(tmp)+line_len);
        }
        strcat(block_of_operations->operations[block_of_operations->last_one], line);

    }
    fclose(in_file);

    add_block(array_of_blocks, block_of_operations);

    return array_of_blocks->last_one;
}

int get_number_of_operations(block * block_of_operations){
    return block_of_operations->last_one + 1;
}

void delete_operation(block * block_of_operations, int operation_id) {
    free(block_of_operations->operations[operation_id]);
    while(operation_id != block_of_operations->last_one) {
        block_of_operations->operations[operation_id] = block_of_operations->operations[operation_id+1];
        operation_id++;
    }
    block_of_operations->last_one--;
}

void delete_block(main_array * array_of_blocks, int block_id) {

    for(int i = 0; i < array_of_blocks->blocks[block_id]->size; i++) {
        free(array_of_blocks->blocks[block_id]->operations[i]);
    }

    free(array_of_blocks->blocks[block_id]->operations);

    free(array_of_blocks->blocks[block_id]);
    while(block_id != array_of_blocks->last_one) {
        array_of_blocks->blocks[block_id] = array_of_blocks->blocks[block_id+1];
        block_id++;
    }

    array_of_blocks->last_one--;
}

pair create_pair(char * element) {
    pair pair_of_files;
    pair_of_files.file1 = strtok(element, ":");
    pair_of_files.file2 = strtok(NULL, ":'");
    return pair_of_files;
}

pair * create_pair_sequence(char **list, int list_size) {
    pair * file_pairs = calloc(list_size, sizeof(pair));
    for(int i=0; i<list_size; i++){
        file_pairs[i] = create_pair(list[i]);
    }
    return file_pairs;
}

void print_pair_sequence(pair *file_pairs, int size) {
    for(int i = 0; i<size; i++){
        printf("Pair: %s, %s\n", file_pairs[i].file1, file_pairs[i].file2);
    }
}

void print_main_array(main_array * array_of_blocks){

    printf("Main array of size %d\n", array_of_blocks->size);
    printf("Current pointer location %d\n", array_of_blocks->last_one);
    for(int i = 0; i<=array_of_blocks->last_one; i++){
        printf("Block number %d of size %d\n", i, array_of_blocks->blocks[i]->last_one+1);
        for(int j = 0; j<=array_of_blocks->blocks[i]->last_one; j++) {
            printf("Operation number %d\n%s", j, array_of_blocks->blocks[i]->operations[j]);
        }
    }
}

void free_main_array(main_array * array_of_blocks){
    for(int i = 0; i<=array_of_blocks->last_one; i++){
        for(int j = 0; j<array_of_blocks->blocks[i]->size; j++){
            free(array_of_blocks->blocks[i]->operations[j]);
        }
        free(array_of_blocks->blocks[i]->operations);
        free(array_of_blocks->blocks[i]);
    }
    free(array_of_blocks->blocks);
    free(array_of_blocks);
}
