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