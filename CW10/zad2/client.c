//
// Created by werka on 5/18/20.
//
#include "header.h"

int local_connection;
char * name;
char * server_path;
int port_number;

int server_sock_fd;
char move[2];
pthread_t reading_thread;
int server_alive = 1;


void connect_to_server() {
    if(local_connection) {
        struct sockaddr_un server_sock;
        server_sock.sun_family = AF_UNIX;
        strcpy(server_sock.sun_path, server_path);

        server_sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if(server_sock_fd==-1) error("Problem with socket!");

        struct sockaddr_un client_sock;
        client_sock.sun_family = AF_UNIX;
        strcpy(client_sock.sun_path, name);

        if(bind(server_sock_fd, (struct sockaddr*) &client_sock, sizeof(client_sock)) == -1) error("Unix binding problem!");

        if(connect(server_sock_fd, (struct sockaddr*) &server_sock, sizeof(server_sock)) == -1) error("Connection problem!");

    }
    else {
        struct sockaddr_in server_sock;
        server_sock.sin_family = AF_INET;
        server_sock.sin_port = htons(port_number);
        server_sock.sin_addr.s_addr = inet_addr(server_path);

        if(server_sock.sin_addr.s_addr==-1) error("Wrong IPV4 address!");

        server_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(server_sock_fd==-1) error("Problem with socket!");

        struct sockaddr_in client_sock;
        client_sock.sin_family = AF_INET;
        client_sock.sin_port = 0;
        client_sock.sin_addr.s_addr = inet_addr(server_path);

        if(bind(server_sock_fd, (struct sockaddr*) &client_sock, sizeof(client_sock)) == -1) error("Inet binding problem!");

        if(connect(server_sock_fd, (struct sockaddr*) &server_sock, sizeof(server_sock)) == -1) error("Connection problem!");

    }

}

void close_connection() {
    if(server_alive==1) {
        send_message(server_sock_fd, LOGOUT, "", name);
    }

    if(shutdown(server_sock_fd, SHUT_RDWR) == -1) error("Problem with shutdown!");

    if(close(server_sock_fd) == -1) error("Problem with closing socket!");

    if(local_connection) {
        if (unlink(name) == -1) error("Problem with socket path unlink!");
    }

    exit(0);
}

void * read_input(){
    char move_[2];
    printf("Enter move: ");
    scanf("%s", move_);

    while(atoi(move_)>9 || atoi(move_)<1){
        printf("Wrong input!\nEnter move: ");
        scanf("%s", move_);
    }
    move[0] = move_[0];
    move[1] = move_[1];
    pthread_exit((void *) 0);
}

void wait_for_input(){
    move[0] = '0';
    pthread_attr_t tattr;

    // set the thread detach state
    if(pthread_attr_init(&tattr)==-1) error("Problem with attributes!");
    if(pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED) != 0) error("Cannot detach thread!");

    if(pthread_create(&reading_thread, &tattr, read_input, NULL) == -1) error("Error with thread creation!");

    message* new_message;
    while(move[0] == '0') {
        new_message = read_message(server_sock_fd, 0); // not waiting read
        if(new_message != NULL) {
            if(new_message->type == PING) {
                printf("Received PING\n");
                send_message(server_sock_fd, PING, "", name);
            }
            else if(new_message->type == SERVER_DIED){
                printf("Server died\n");
                server_alive = 0;
                close_connection();
            }
            free(new_message);
        }
    }
}

int main(int argc, char ** argv) {
    if (argc <= 1) {
        printf("You specified too few arguments!");
        return 1;
    }

    local_connection = strcmp("LOCAL", argv[1]) == 0 ? 1 : 0;

    if ((argc <= 3 && local_connection) || (argc<=4 && !local_connection)) {
        printf("You specified too few arguments!");
        return 1;
    }

    name = argv[2];
    server_path = argv[3];

    if(!local_connection) port_number = atoi(argv[4]);

    signal(SIGINT, close_connection);

    connect_to_server();

    send_message(server_sock_fd, LOGIN_ATTEMPT, name, name);
    message* new_message = read_message(server_sock_fd, 1);
    printf("Server answer: %s\n", new_message->message_data);

    if(new_message->type == LOGIN_APPROVAL){

        int logged_in = 1;
        while(logged_in){
            free(new_message);
            new_message = read_message(server_sock_fd, 1);
            if (new_message->type == PING){
                printf("Received PING\n");
                send_message(server_sock_fd, PING, "", name);
            }
            else if(new_message->type == SERVER_DIED){
                printf("Server died\n");
                server_alive = 0;
                close_connection();
            }
            else if(new_message->type == NO_FREE_PLAYER) printf("Waiting for a second player\n");
            else if(new_message->type == GAME_SIGN){
                printf("Game started\nYou are %s\nO starts\n", new_message->message_data);
                printf("---------\n_________\n_________\n_________\n---------\n");
                int my_turn = 1;

                if(strcmp(new_message->message_data, "O")==0){
                    wait_for_input();
                    send_message(server_sock_fd, GAME_MOVE, move, name);
                    my_turn = 0;
                }

                while(1){
                    free(new_message);
                    new_message = read_message(server_sock_fd, 1);
                    if (new_message->type == PING){
                        printf("Received PING\n");
                        send_message(server_sock_fd, PING, "", name);
                    }
                    else if(new_message->type == SERVER_DIED){
                        printf("Server died\n");
                        server_alive = 0;
                        close_connection();
                    }
                    else if(new_message->type == MSG) printf("%s\n", new_message->message_data);
                    else if(new_message->type == GAME_MOVE){
                        printf("%s\n---------\n", new_message->message_data);
                        if(my_turn==1){
                            wait_for_input();
                            send_message(server_sock_fd, GAME_MOVE, move, name);
                            my_turn = 0;
                        } else{
                            my_turn = 1;
                        }
                    }
                    else if(new_message->type == GAME_TIE){
                        printf("%s\n", new_message->message_data);
                        logged_in = 0;
                        break;
                    }
                    else if(new_message->type == GAME_WINNER){
                        printf("Game won by %s\n", new_message->message_data);
                        logged_in = 0;
                        break;
                    }
                }

            }
        }
    }
    free(new_message);
    sleep(5);
    close_connection();

    return 0;
}