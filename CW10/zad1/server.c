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

    inet_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(inet_sock_fd == -1) error("Problem with socket file descriptor!");
    if(bind(inet_sock_fd, (struct sockaddr *)&inet_sock, sizeof(inet_sock)) == -1) error("Problem with binding!");
    if (listen(inet_sock_fd, MAX_CLIENTS_NUMBER) == -1) error("Problem with listening set!");

    //unix socket

    unix_sock.sun_family = AF_UNIX;
    strcpy(unix_sock.sun_path, socket_path);

    unix_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(inet_sock_fd == -1) error("Problem with socket file descriptor!");
    if(bind(unix_sock_fd, (struct sockaddr *)&unix_sock, sizeof(unix_sock)) == -1) error("Problem with binding!");
    if (listen(unix_sock_fd, MAX_CLIENTS_NUMBER) == -1) error("Problem with listening set!");

}

void close_connection(int fd) {
    if(shutdown(fd, SHUT_RDWR)==-1) error("Problem with socket fd shutdown!");
    if(close(fd)==-1) error("Problem with closing socket fd!");
}

void stop_server() {
    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++) {
        if(clients[i]!=NULL) send_message(clients[i]->fd, SERVER_DIED, "");
    }

    if(pthread_cancel(messaging_thread)==-1) error("Problem with thread cancellation!");
    if(pthread_cancel(ping_thread)==-1) error("Problem with thread cancellation!");

    close_connection(inet_sock_fd);
    close_connection(unix_sock_fd);

    if(unlink(socket_path)==-1) error("Problem with socket path unlink!");

    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++) free(games[i]);
    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++) free(clients[i]);

    exit(0);
}

int add_client_entry(int fd, char * name){
    int next_free_id = -1;
    for(int i = 0; i<MAX_CLIENTS_NUMBER; i++){
        if(clients[i]==NULL && next_free_id == -1) next_free_id = i;
        if(clients[i]!=NULL &&strcmp(clients[i]->name, name)==0) return -1;
    }
    if(next_free_id==-1) return -2;
    clients[next_free_id] = create_client_entry(fd, name);
    return next_free_id;
}

int get_new_login(int sock_fd) {
    int new_sock_fd = accept(sock_fd, NULL, NULL);
    if (new_sock_fd==-1) error("Problem with accept function!");

    message* new_message = read_message(new_sock_fd, 1);
    printf("Received name %s\n", new_message->message_data);

    int registered_id = add_client_entry(new_sock_fd, new_message->message_data);

    if(registered_id == -1) {
        printf("Login attempt failed\n");
        send_message(new_sock_fd, LOGIN_REJECTION, "Client name exists");
        close_connection(new_sock_fd);
    } else if(registered_id == -2){
        printf("Login attempt failed\n");
        send_message(new_sock_fd, LOGIN_REJECTION, "Too many clients right now");
        close_connection(new_sock_fd);
    } else {
        printf("Login accepted\n");
        send_message(new_sock_fd, LOGIN_APPROVAL, "Logged in successfully");
    }
    free(new_message);
    return registered_id;
}

void remove_login(int fd){
    for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
        if (clients[i] && clients[i]->fd == fd) {
            clients[i] = NULL;
        }
    }
}

int random_int(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void match_players(int registered_id){
    if(player_waiting==-1){
        send_message(clients[registered_id]->fd, NO_FREE_PLAYER, "Wait for new player");
        player_waiting = registered_id;
        printf("Player %d is waiting for a new player\n", registered_id);
    } else {
        game * new_game = get_new_game(registered_id, player_waiting);
        games[registered_id] = new_game;
        games[player_waiting] = new_game;
        if(random_int(0, 1)==0){
            send_message(clients[registered_id]->fd, GAME_SIGN, "O");
            signs[registered_id] ='O';
            send_message(clients[player_waiting]->fd, GAME_SIGN, "X");
            signs[player_waiting] ='X';
        }
        else{
            send_message(clients[player_waiting]->fd, GAME_SIGN, "O");
            signs[player_waiting] ='O';
            send_message(clients[registered_id]->fd, GAME_SIGN, "X");
            signs[registered_id] ='X';
        }
        player_waiting = -1;

    }
}

void * process_messaging() {
    pollfd fds[MAX_CLIENTS_NUMBER+2];

    for(int i = 0; i<MAX_CLIENTS_NUMBER+2; i++){
        fds[i].events = POLLIN;
    }

    fds[MAX_CLIENTS_NUMBER].fd = inet_sock_fd;
    fds[MAX_CLIENTS_NUMBER+1].fd = unix_sock_fd;

    player_waiting = -1;

    while(1){
        pthread_mutex_lock(&mutex);
        for(int i = 0; i<MAX_CLIENTS_NUMBER; i++){
            if(clients[i]==NULL) fds[i].fd = -1;
            else fds[i].fd = clients[i]->fd;

            fds[i].revents = 0;
        }
        pthread_mutex_unlock(&mutex);

        fds[MAX_CLIENTS_NUMBER].revents = 0;
        fds[MAX_CLIENTS_NUMBER+1].revents = 0;

        if(poll(fds, MAX_CLIENTS_NUMBER+2, -1)==-1) error("Problem with polling!"); //wait with no time limit

        pthread_mutex_lock(&mutex);
        for(int i = 0; i<MAX_CLIENTS_NUMBER+2; i++){
            if(i<MAX_CLIENTS_NUMBER && clients[i]==NULL) continue;

            if(fds[i].revents & POLLIN) {

                if(fds[i].fd == unix_sock_fd || fds[i].fd == inet_sock_fd){
                    int registered_id = get_new_login(fds[i].fd);
                    if(registered_id>=0) {
                        printf("Client registered at index %d\n", registered_id);
                        match_players(registered_id);
                    }
                }
                else{
                    message * new_message = read_message(fds[i].fd, 1);
                    if(new_message->type == LOGOUT) {
                        printf("Client %s is logged out\n", clients[i]->name);
                        close_connection(fds[i].fd);
                        remove_login(fds[i].fd);
                    } else if(new_message->type ==  PING) {
                        clients[i]->active = 1;
                    } else if(new_message->type == GAME_MOVE) {
                        int game_move = make_move(atoi(new_message->message_data), games[i], signs[i]);
                        if(game_move==-1) send_message(clients[i]->fd, MSG, "Position taken, you lost turn");

                        char* winner = check_winner(games[i]);

                        if(strcmp(winner, "_")==0) {
                            char *board_state = get_board_state(games[i]);
                            send_message(clients[games[i]->player1]->fd, GAME_MOVE, board_state);
                            send_message(clients[games[i]->player2]->fd, GAME_MOVE, board_state);
                            free(board_state);
                        }
                        else if(strcmp(winner, "T")==0) {
                            send_message(clients[games[i]->player1]->fd, GAME_TIE, "Game ended with a tie");
                            send_message(clients[games[i]->player2]->fd, GAME_TIE, "Game ended with a tie");
                            break;
                        }
                        else {
                            send_message(clients[games[i]->player1]->fd, GAME_WINNER, winner);
                            send_message(clients[games[i]->player2]->fd, GAME_WINNER, winner);
                            break;
                        }

                    }
                    free(new_message);
                }
            }
            else if(fds[i].revents & POLLHUP) {
                close_connection(fds[i].fd);
                remove_login(fds[i].fd);
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
                send_message(clients[i]->fd, PING, "");
            }
        }
        pthread_mutex_unlock(&mutex);

        sleep(PING_WAIT);

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
            if (clients[i] != NULL && clients[i]->active == 0) {
                printf("Client unresponsive\n");
                close_connection(clients[i]->fd);
                remove_login(clients[i]->fd);
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
