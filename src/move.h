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