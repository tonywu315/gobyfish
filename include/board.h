#ifndef BOARD_H
#define BOARD_H

#include "constants.h"

void init_board();
void start_board();
void print_board(int score);
void load_fen(const char *fen);
void load_pgn(const char *pgn);

#endif
