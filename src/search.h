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

#ifndef SEARCH_H
#define SEARCH_H

/* Includes */

#include "global.h"
#include "board.h"
#include "move.h"
#include "gen.h"
#include "evaluate.h"
#include "transposition.h"

/* Definitions */

#define START 0
#define TIMENOW 1
#define MAX_DEPTH 100

#define MAX_HISTORY 400
#define MIN_HISTORY -400

/* Functions */

void init_pv();
void init_history();

int timer(int limit_ms, bool action);

SMove iterate_search(int time);

#endif // SEARCH_H