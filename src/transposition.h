/*  Carpathian is an UCI chess playing program
    Copyright (C) 2026 Cheyetch

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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