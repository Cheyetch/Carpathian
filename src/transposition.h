#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

/* Includes */

#include "global.h"
#include "board.h"
#include "move.h"
#include "search.h"

/* Definitions */

#define TT_EXACT 1
#define TT_ALPHA 2
#define TT_BETA  3

#define TT_SIZE (1 << 22)
#define TT_MASK 0x3fffff
extern STTEntry tt[TT_SIZE]; // 4 Million entries = 75 MiB

/* Functions */

void init_tt();
int tt_probe(U8 depth, int alpha, int beta, SMove *bestmove);
void tt_save(U8 depth, int val, int flag, SMove bestmove);

#endif // TRANSPOSITION_H