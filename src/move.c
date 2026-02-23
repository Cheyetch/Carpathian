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

#include "move.h"

SMove encode_move(U8 from, U8 to, U8 special, U8 moved, U8 captured) {
    SMove move;
    move.move = from | (to << 6) | (special << 12);
    move.moved = moved;
    move.captured = captured;

    return move;
}

void do_move(SMove move, SState *s) {
    U8 from = FROM(move.move);
    U8 to = TO(move.move);
    U8 special = SPECIAL(move.move);

    /* State updating phase */

    // Set a state "s" for unmaking this move
    // This is done to revrese irreversible actions
    s->castling = game.castling;
    s->epTarget = game.epTarget;
    s->fifty = game.fifty;

    // After making a move, the current hash is stored in the repetition table
    game.repetitionTable[game.ply] = game.hash;
    update_hash(); // First update, decouples castling and ep to the hash

    // Updating ply and side to move
    game.ply++;
    game.isWhite = !game.isWhite;
    game.hash ^= zobrist.isBlack; // Update the side moving hash

    // Updating fifty move clock
    game.fifty++;
    if ((move.moved == wPawn) || (move.moved == bPawn) || (move.move & CAPTUREFLAG)) game.fifty = 0;

    // Updating ep square
    game.epTarget = NULLSQUARE;
    if (special == push) {
        game.epTarget = to + ((move.moved == wPawn) ? -8:8);
    }

    // Updating castling rights
    // DO NOT CHANGE TO ELSE IFS - VERY WEIRD BUG
    if (move.moved == wKing) game.castling &= (U8)~(wk | wq);
    if (move.moved == bKing) game.castling &= (U8)~(bk | bq);
    if ((from == a1) || (to == a1)) game.castling &= (U8)~wq;
    if ((from == h1) || (to == h1)) game.castling &= (U8)~wk;
    if ((from == a8) || (to == a8)) game.castling &= (U8)~bq;
    if ((from == h8) || (to == h8)) game.castling &= (U8)~bk;

    update_hash(); // Second update, restores castling and ep to the hash

    /* Moves and special moves phase*/

    // Moving the piece
    clear_square(to);
    clear_square(from);
    fill_square(to, move.moved);

    // Castling moves
    if (special == kCastle) {
        clear_square(to+1);
        // Trick used to remove branching when checking for color during castling
        fill_square(to-1, move.moved-2);
    }
    else if (special == qCastle) {
        clear_square(to-2);
        fill_square(to+1, move.moved-2);
    }

    // Handling ep
    else if (special == ep) {
        clear_square(to + ((move.moved == wPawn) ? -8:8));
    }

    // Handling promotions
    else if ((special == qPromo) || (special == qPromoCapture)) {
        clear_square(to);
        // The same dirty trick used in castling
        fill_square(to, move.moved+4); // Queen
    }
    else if ((special == rPromo) || (special == rPromoCapture)) {
        clear_square(to);
        fill_square(to, move.moved+3); // Rook
    }
    else if ((special == bPromo) || (special == bPromoCapture)) {
        clear_square(to);
        fill_square(to, move.moved+2); // Bishop
    }
    else if ((special == nPromo) || (special == nPromoCapture)) {
        clear_square(to);
        fill_square(to, move.moved+1); // Knight
    }
}

void undo_move(SMove move, SState *s) {
    U8 from = FROM(move.move);
    U8 to = TO(move.move);
    U8 special = SPECIAL(move.move);

    /* State updating phase */

    // Restore irreversible aspects from a state "s"
    game.castling = s->castling;
    game.epTarget = s->epTarget;
    game.fifty = s->fifty;

    // Updating ply and side to move
    game.ply--;
    game.isWhite = !game.isWhite;
    game.hash ^= zobrist.isBlack; // Update the side moving hash

    /* Moves and special moves phase*/

    // Moving the piece
    fill_square(from, move.moved); // Restore the moved piece
    clear_square(to);
    fill_square(to, move.captured); // Restore the captured piece

    // Castling moves
    if (special == kCastle) {
        clear_square(to-1);
        fill_square(to+1, move.moved-2);
    }
    else if (special == qCastle) {
        clear_square(to+1);
        fill_square(to-2, move.moved-2);
    }

    // Handling ep
    else if (special == ep) {
        clear_square(to); // The pawn captured trough ep was restored at the ep target, which is wrong
        fill_square(to + ((move.moved == wPawn) ? -8:8), move.captured);
    }

    // No reason to handle promotions, the end square is cleared and the moved piece was a pawn
    // Empty the slot in the repetition table
    game.hash = game.repetitionTable[game.ply];
    game.repetitionTable[game.ply] = 0;
}

void read_move(char *move) {
    U8 from = 8 * (move[1] - '1') + move[0] - 'a';
    U8 to = 8 * (move[3] - '1') + move[2] - 'a';
    U8 special = quiet;
    U8 moved = game.square[from];
    U8 captured = game.square[to];

    // Captures
    if (captured != EMPTY) special = capture;

    // Pushes and ep
    if ((moved == wPawn) || (moved == bPawn)) {
        if ((to == (from + 16)) || (to == (from - 16))) special = push;
        if (to == game.epTarget) special = ep;
    }

    // Castles
    if ((moved == wKing) || (moved == bKing)) {
        if (to == (from + 2)) special = kCastle;
        if (to == (from - 2)) special = qCastle;
    }

    // Promotions
    switch (move[4]) {
        case 'q': special += qPromo; break;
        case 'r': special += rPromo; break;
        case 'b': special += bPromo; break;
        case 'n': special += nPromo; break;
        case '\0': 
        default: break;
    }

    static SState s;
    do_move(encode_move(from, to, special, moved, captured), &s);
}