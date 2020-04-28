//
// Created by werka on 4/27/20.
//

#ifndef SYSOPY_HEADER_H
#define SYSOPY_HEADER_H
#define MAX_ORDERS 4
#define W_1 10
#define W_2 10
#define W_3 10
#define SEM_TO_RECEIVE "/to_receive"
#define SEM_ID_TO_RECEIVE "/id_to_receive"
#define SEM_TO_PREPARE "/to_prepare"
#define SEM_ID_TO_PREPARE "/id_to_prepare"
#define SEM_TO_SEND "/to_send"
#define SEM_ID_TO_SEND "/id_to_send"
#define SEM_MEM_ACCESS "/mem_access"
#define ORDERS "/orders"

typedef struct orders{
    int parcels[MAX_ORDERS];
} orders;

#endif //SYSOPY_HEADER_H
