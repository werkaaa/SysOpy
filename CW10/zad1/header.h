//
// Created by werka on 5/20/20.
//

#ifndef SYSOPY_MESSAGING_H
#define SYSOPY_MESSAGING_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_addr
#include <sys/un.h>
#include <netdb.h>
#include <sys/types.h> //accept
#include <sys/socket.h> //accept
#include <poll.h> //poll
#include <pthread.h> //threads and mutex
#include <signal.h>
#include <unistd.h>

#define MAX_NAME_LEN 32
#define MAX_MSG_LEN 40
#define MAX_CLIENTS_NUMBER 20

#define PING_BREAK 30
#define PING_WAIT 30

typedef struct pollfd pollfd;

typedef struct client {
    int fd;
    char name[MAX_NAME_LEN];
    int  active;
} client;

typedef enum MSG_TYPE{
    MSG,
    LOGIN_ATTEMPT,
    LOGIN_APPROVAL,
    LOGIN_REJECTION,
    LOGOUT,
    GAME_MOVE,
    GAME_SIGN,
    NO_FREE_PLAYER,
    PING,
    GAME_WINNER,
    GAME_TIE,
    SERVER_DIED
} MSG_TYPE;

typedef struct message {
    MSG_TYPE type;
    char message_data[MAX_MSG_LEN-1];
} message;

typedef struct game {
    char board[9];
    int player1;
    int player2;
} game;

// common utils
void error(char* msg);

// messaging functions
client * create_client_entry(int fd, char * name);
void send_message(int sock_fd, MSG_TYPE type, char * message_data);
message * read_message(int sock_fd, int block);

// game functions
game* get_new_game(int player1, int player2);
char * get_board_state(game * game_state);
int make_move(int pos, game* game_state, char sign);
char* check_winner(game * game_state);


#endif //SYSOPY_MESSAGING_H
