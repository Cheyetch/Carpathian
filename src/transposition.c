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

#include "transposition.h"

STTEntry tt[TT_SIZE];

void init_tt() {
    SMove NULLMOVE;
    NULLMOVE.captured = EMPTY;
    NULLMOVE.moved = EMPTY;
    NULLMOVE.move = 0;

    for (int i = 0; i < (1 << 22); i++) {
        tt[i].hash = 0;
        tt[i].val = 0;
        tt[i].bestmove = NULLMOVE;
        tt[i].depth = 0;
        tt[i].flag = TT_EXACT;
    }
}

int tt_probe(U8 depth, int alpha, int beta, SMove *best) {
    STTEntry *entry = &tt[game.hash & TT_MASK];

    if (entry->hash == game.hash) {
        *best = entry->bestmove;

        // Subtract the current ply from the mate score,
        // we are returning the mate score + delta ply (<= 0)
        int val = entry->val;
        if (val > 90000000)
            val -= game.ply;
        if (val < -90000000)
            val += game.ply;

        if (entry->depth >= depth) {
            if (entry->flag == TT_EXACT) return val;
            if ((entry->flag == TT_ALPHA) && (val <= alpha)) return alpha;
            if ((entry->flag == TT_BETA) && (val >= beta)) return beta;
        }
    }

    return NULLSCORE;
}

void tt_save(U8 depth, int val, int flag, SMove best) {
    if (timer(0, TIMENOW)) return;

    STTEntry *entry = &tt[game.hash & TT_MASK];

    if ((entry->hash == game.hash) && (entry->depth > depth)) return;

    // In the TT we store depth to mate, not the ply
    if (val > 90000000 ) {
        int m_ply = WIN - val;
        int distance = m_ply - game.ply;
        val = WIN - distance;
    }
    if (val < -90000000) {
        int m_ply = val - LOS;
        int distance = m_ply - game.ply;
        val = LOS + distance;
    }

    entry->hash = game.hash;
    entry->val = val;
    entry->bestmove = best;
    entry->depth = depth;
    entry->flag = flag;
}