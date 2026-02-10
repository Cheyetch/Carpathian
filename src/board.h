#ifndef BOARD_H
#define BOARD_H

/* Includes */

#include "global.h"

/* Functions */

U64 rand64();
void init_hashes();
void update_hash();

void fill_square(U8 sq, U8 piece);
void clear_square(U8 sq);

void clear_board();
void load_fen(char fen[]);
void load_startpos();

int three_fold();

void print_board();
void print_move(SMove move);

#endif // BOARD_H