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