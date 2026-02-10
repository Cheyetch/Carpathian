#ifndef MOVE_H
#define MOVE_H

/* Includes */

#include "global.h"
#include "board.h"

/* Functions */

SMove encode_move(U8 from, U8 to, U8 special, U8 moved, U8 captured);
void read_move(char *move);

void do_move(SMove move, SState *s);
void undo_move(SMove move, SState *s);

#endif // MOVE_H