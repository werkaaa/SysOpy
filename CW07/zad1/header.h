//
// Created by werka on 4/27/20.
//

#ifndef SYSOPY_HEADER_H
#define SYSOPY_HEADER_H
#define MAX_ORDERS 5
#define W_1 10
#define W_2 10
#define W_3 10

typedef struct orders{
    int parcels[MAX_ORDERS];
} orders;

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

#endif //SYSOPY_HEADER_H
