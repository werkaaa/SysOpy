//
// Created by werka on 5/18/20.
//
#include "header.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

client* clients[MAX_CLIENTS_NUMBER];
game * games[MAX_CLIENTS_NUMBER];
char signs[MAX_CLIENTS_NUMBER];

int port_number;
char* socket_path;

struct sockaddr_in inet_sock;
struct sockaddr_un unix_sock;

int inet_sock_fd;
int unix_sock_fd;

pthread_t messaging_thread;
pthread_t ping_thread;

int player_waiting;

void start_server(){

    //internet socket

    struct hostent* host_ent = gethostbyname("localhost");
    if(host_ent==NULL) error("problem with getting host name!");

    inet_sock.sin_family = AF_INET;
    inet_sock.sin_port = htons(port_number);
    inet_sock.sin_addr = *(struct in_addr*) host_ent->h_addr;

    inet_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(inet_sock_fd == -1) error("Problem with socket file descriptor!");
    if(bind(inet_sock_fd, (struct sockaddr *)&inet_sock, sizeof(inet_sock)) == -1) error("Problem with inet binding!");

    //unix socket

    unix_sock.sun_family = AF_UNIX;
    strcpy(unix_sock.sun_path, socket_path);

    unix_sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(unix_sock_fd == -1) error("Problem with socket file descriptor!");
    if(bind(unix_sock_fd, (struct sockaddr *)&unix_sock, sizeof(unix_sock)) == -1) error("Problem with unix binding!");

}

void close_connection(int fd) {
    if(close(fd)==-1) error("Problem with closing socket fd!");
}

void stop_server() {
    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++) {
        if(clients[i]!=NULL) send_message_to(clients[i]->fd, clients[i]->address, SERVER_DIED, "");
    }

    if(pthread_cancel(messaging_thread)==-1) error("Problem with thread cancellation!");
    if(pthread_cancel(ping_thread)==-1) error("Problem with thread cancellation!");

    if(close(inet_sock_fd)==-1) error("Problem with closing socket fd!");
    if(close(unix_sock_fd)==-1) error("Problem with closing socket fd!");

    if(unlink(socket_path)==-1) error("Problem with socket path unlink!");

    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++) free(games[i]);
    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++) free(clients[i]);

    exit(0);
}

int add_client_entry(int fd, char * name, struct sockaddr* address){
    int next_free_id = -1;
    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++){
        if(clients[i]==NULL && next_free_id == -1) next_free_id = i;
        if(clients[i]!=NULL &&strcmp(clients[i]->name, name)==0) return -1;
    }
    if(next_free_id==-1) return -2;
    clients[next_free_id] = create_client_entry(fd, name, address);
    return next_free_id;
}

void remove_id(int id){
    clients[id] = NULL;
    games[id] = NULL;
    signs[id] = '_';
}

int random_int(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void match_players(int registered_id){
    if(player_waiting==-1){
        send_message_to(clients[registered_id]->fd, clients[registered_id]->address, NO_FREE_PLAYER, "Wait for new player");
        player_waiting = registered_id;
        printf("Player %d is waiting for a new player\n", registered_id);
    } else {
        game * new_game = get_new_game(registered_id, player_waiting);
        games[registered_id] = new_game;
        games[player_waiting] = new_game;
        if(random_int(0, 1)==0){
            send_message_to(clients[registered_id]->fd, clients[registered_id]->address, GAME_SIGN, "O");
            signs[registered_id] ='O';
            send_message_to(clients[player_waiting]->fd, clients[player_waiting]->address, GAME_SIGN, "X");
            signs[player_waiting] ='X';
        }
        else{
            send_message_to(clients[player_waiting]->fd, clients[player_waiting]->address, GAME_SIGN, "O");
            signs[player_waiting] ='O';
            send_message_to(clients[registered_id]->fd, clients[registered_id]->address, GAME_SIGN, "X");
            signs[registered_id] ='X';
        }
        player_waiting = -1;

    }
}

int get_id_from_name(char * name){
    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++){
        if(clients[i]!=NULL && strcmp(name, clients[i]->name)==0)
            return i;
    }
    return -1;
}

void * process_messaging() {
    pollfd fds[2];

    fds[0].fd = unix_sock_fd;
    fds[0].events = POLLIN;

    fds[1].fd = inet_sock_fd;
    fds[1].events = POLLIN;

    player_waiting = -1;

    while(1){
        fds[0].revents = 0;
        fds[1].revents = 0;

        if(poll(fds, 2, -1)==-1) error("Problem with polling!"); //wait with no time limit

        pthread_mutex_lock(&mutex);
        for(int i = 0; i<2; i++){
            if(fds[i].revents & POLLIN) {
                struct sockaddr* address = (struct sockaddr*) calloc(1, sizeof(struct sockaddr));
                socklen_t len = sizeof(&address);
                message * new_message = read_message_from(fds[i].fd, address, &len);
                int id = get_id_from_name(new_message->sender);

                if(new_message->type == LOGIN_ATTEMPT) {

                    int registered_id = add_client_entry(fds[i].fd, new_message->message_data, address);

                    if(registered_id == -1) {
                        printf("Login attempt failed\n");
                        send_message_to(fds[i].fd, address, LOGIN_REJECTION, "Client name exists");
                    } else if(registered_id == -2){
                        printf("Login attempt failed\n");
                        send_message_to(fds[i].fd, address, LOGIN_REJECTION, "Too many clients right now");
                    } else {
                        printf("Login accepted\nClient registered at index %d\n", registered_id);
                        send_message_to(fds[i].fd, address, LOGIN_APPROVAL, "Logged in successfully");
                        match_players(registered_id);
                    }

                }
                else if(new_message->type == LOGOUT) {
                    printf("Client %s is logged out\n", clients[id]->name);
                    remove_id(id);
                } else if(new_message->type ==  PING) {
                    clients[id]->active = 1;
                } else if(new_message->type == GAME_MOVE) {
                    int game_move = make_move(atoi(new_message->message_data), games[id], signs[id]);
                    if(game_move==-1) send_message_to(clients[id]->fd, address, MSG, "Position taken, you lost turn");

                    char* winner = check_winner(games[i]);
                    char *board_state = get_board_state(games[i]);
                    if(strcmp(winner, "_")==0) {
                        send_message_to(clients[games[id]->player1]->fd, clients[games[id]->player1]->address, GAME_MOVE, board_state);
                        send_message_to(clients[games[id]->player2]->fd, clients[games[id]->player2]->address, GAME_MOVE, board_state);
                        free(board_state);
                    }
                    else if(strcmp(winner, "T")==0) {
                        send_message_to(clients[games[id]->player1]->fd, clients[games[id]->player1]->address, MSG, board_state);
                        send_message_to(clients[games[id]->player2]->fd, clients[games[id]->player2]->address, MSG, board_state);
                        send_message_to(clients[games[id]->player1]->fd, clients[games[id]->player1]->address, GAME_TIE, "Game ended with a tie");
                        send_message_to(clients[games[id]->player2]->fd, clients[games[id]->player2]->address, GAME_TIE, "Game ended with a tie");
                        break;
                    }
                    else {
                        send_message_to(clients[games[id]->player1]->fd, clients[games[id]->player1]->address, MSG, board_state);
                        send_message_to(clients[games[id]->player2]->fd, clients[games[id]->player2]->address, MSG, board_state);
                        send_message_to(clients[games[id]->player1]->fd, clients[games[id]->player1]->address, GAME_WINNER, winner);
                        send_message_to(clients[games[id]->player2]->fd, clients[games[id]->player2]->address, GAME_WINNER, winner);
                        break;
                    }

                }
                free(new_message);
            }

        }
        pthread_mutex_unlock(&mutex);

    }
    pthread_exit((void *) 0);
}

void * process_ping(){
    while(1) {
        sleep(PING_BREAK);

        pthread_mutex_lock(&mutex);
        printf("Pinging clients\n");
        for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
            if (clients[i] != NULL) {
                clients[i]->active = 0;
                send_message_to(clients[i]->fd, clients[i]->address, PING, "");
            }
        }
        pthread_mutex_unlock(&mutex);

        sleep(PING_WAIT);

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
            if (clients[i] != NULL && clients[i]->active == 0) {
                printf("Client unresponsive\n");
                remove_id(get_id_from_name(clients[i]->name));
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit((void *) 0);
}


int main(int argc, char ** argv) {
    if (argc <= 2) {
        printf("You specified too few arguments!");
        return 1;
    }

    port_number = atoi(argv[1]);
    socket_path = argv[2];

    signal(SIGINT, stop_server);

    start_server();
    srand(time(NULL));

    if(pthread_create(&messaging_thread, NULL, process_messaging, NULL) == -1) error("Problem with pthread_create function!");
    if(pthread_create(&ping_thread, NULL, process_ping, NULL) == -1) error("Problem with pthread_create function!");

    if(pthread_join(messaging_thread, NULL) == -1) error("Problem with pthread_join function!");
    if(pthread_join(ping_thread, NULL) == -1) error("Problem with pthread_join function!");

    stop_server();

    return 0;
}
