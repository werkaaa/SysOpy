//
// Created by werka on 5/20/20.
//
#include "header.h"

game* get_new_game(int player1, int player2){
    game * new_game = (game*)calloc(1, sizeof(game));
    new_game->player1 = player1;
    new_game->player2 = player2;
    for(int i = 0; i<9; i++) new_game->board[i] = '_';
    return new_game;
}

char * get_board_state(game * game_state){
    char * board_string = (char *)calloc(13, sizeof(char));
    for(int i = 0; i<9; i++)
        board_string[i+i/3] = game_state->board[i];

    board_string[3] = '\n';
    board_string[7] = '\n';
    board_string[12] = '\n';
    return board_string;
}

int make_move(int pos, game* game_state, char sign){
    if(game_state->board[pos-1]!='_') return -1;
    game_state->board[pos-1] = sign;
    return 0;
}

char* check_winner(game * game_state){

    int winning_states[8][3] = {{0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 4, 8}, {2, 4, 6}};
    int full_board = 1;
    for(int i = 0; i<8; i++){
        if(game_state->board[winning_states[i][0]]=='O' && game_state->board[winning_states[i][1]]=='O' && game_state->board[winning_states[i][2]]=='O')
            return "O";
        else if(game_state->board[winning_states[i][0]]=='X' && game_state->board[winning_states[i][1]]=='X' && game_state->board[winning_states[i][2]]=='X')
            return "X";
    }

    for(int i = 0; i<9; i++) {
        if (game_state->board[i] == '_') full_board = 0;
    }

    if(full_board==1) return "T";

    return "_";

}

