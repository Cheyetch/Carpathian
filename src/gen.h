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