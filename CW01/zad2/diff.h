//
// Created by werka on 3/4/20.
//

#ifndef SYSOPY_LIBRARY_H
#define SYSOPY_LIBRARY_H

typedef struct {
    int size;
    int last_one;
    char ** operations;
} block;

typedef struct {
    int size;
    int last_one;
    block ** blocks;
} main_array;

typedef struct {
    char * file1;
    char * file2;
} pair;

main_array * create_main_array(int sequence_size);
pair * create_pair_sequence(char **list, int list_size);
char * compare_pair(pair * pair_to_compare);
int save_block_to_main_array(main_array * array_of_blocks, char * file_name);
int get_number_of_operations(block * block_of_operations);
void delete_block(main_array * array_of_blocks, int block_id);
void delete_operation(block * block_of_operations, int operation_id);

void print_pair_sequence(pair *file_pairs, int size);
void print_main_array(main_array * array_of_blocks);

void free_main_array(main_array * array_of_blocks);

#endif //SYSOPY_LIBRARY_H
