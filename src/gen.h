#ifndef GEN_H
#define GEN_H

/* Includes */

#include "global.h"
#include "board.h"
#include "move.h"

/* Functions */

void init_attacks();
void init_magics();

int is_attacked(int side, int sq, BITBOARD occ, BITBOARD not);
int is_check();

void gen_quiet(SMove moves[], int *n);
void gen_capture(SMove moves[], int *n);
void gen_special(SMove moves[], int *n);

void perf_test(int depth);

#endif // GEN_H