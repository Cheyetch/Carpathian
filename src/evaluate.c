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

#include "evaluate.h"

const int mg_value[6] = {82, 337, 365, 477, 1025, 20000};
const int eg_value[6] = {94, 281, 297, 512,  936, 20000};
const int PMG =  82, PEG = 94;
const int NMG = 337, NEG = 281;
const int BMG = 365, BEG = 297;
const int RMG = 477, REG = 512;
const int QMG =1025, QEG = 936;

// Adding the material values inside the PSQTs saves
// 10 expensive popcount operations every time we eval

// The PSQTs are from file A to H and from rank 8 to rank 1
// black index = piece square
// white index = piece square ^ 56
int mg_pawn_table[64] = {
    PMG+0,    PMG+0,    PMG+0,    PMG+0,    PMG+0,    PMG+0,    PMG+0,    PMG+0,
    PMG+98,   PMG+134,  PMG+61,   PMG+95,   PMG+68,   PMG+126,  PMG+34,   PMG-11,
    PMG-6,    PMG+7,    PMG+26,   PMG+31,   PMG+65,   PMG+56,   PMG+25,   PMG-20,
    PMG-14,   PMG+13,   PMG+6,    PMG+21,   PMG+23,   PMG+12,   PMG+17,   PMG-23,
    PMG-27,   PMG-2,    PMG-5,    PMG+12,   PMG+17,   PMG+6,    PMG+10,   PMG-25,
    PMG-26,   PMG-4,    PMG-4,    PMG-10,   PMG+3,    PMG+3,    PMG+33,   PMG-12,
    PMG-35,   PMG-1,    PMG-20,   PMG-23,   PMG-15,   PMG+24,   PMG+38,   PMG-22,
    PMG+0,    PMG+0,    PMG+0,    PMG+0,    PMG+0,    PMG+0,    PMG+0,    PMG+0,
};

int eg_pawn_table[64] = {
    PEG+0,    PEG+0,    PEG+0,    PEG+0,    PEG+0,    PEG+0,    PEG+0,    PEG+0,
    PEG+178,  PEG+173,  PEG+158,  PEG+134,  PEG+147,  PEG+132,  PEG+165,  PEG+187,
    PEG+94,   PEG+100,  PEG+85,   PEG+67,   PEG+56,   PEG+53,   PEG+82,   PEG+84,
    PEG+32,   PEG+24,   PEG+13,   PEG+5,    PEG-2,    PEG+4,    PEG+17,   PEG+17,
    PEG+13,   PEG+9,    PEG-3,    PEG-7,    PEG-7,    PEG-8,    PEG+3,    PEG-1,
    PEG+4,    PEG+7,    PEG-6,    PEG+1,    PEG+0,    PEG-5,    PEG-1,    PEG-8,
    PEG+13,   PEG+8,    PEG+8,    PEG+10,   PEG+13,   PEG+0,    PEG+2,    PEG-7,
    PEG+0,    PEG+0,    PEG+0,    PEG+0,    PEG+0,    PEG+0,    PEG+0,    PEG+0,
};

int mg_knight_table[64] = {
    NMG-167, NMG-89,  NMG-34,  NMG-49,  NMG+61,  NMG-97,  NMG-15, NMG-107,
    NMG-73,  NMG-41,  NMG+72,  NMG+36,  NMG+23,  NMG+62,  NMG+7,  NMG-17,
    NMG-47,  NMG+60,  NMG+37,  NMG+65,  NMG+84,  NMG+129, NMG+73, NMG+44,
    NMG-9,   NMG+17,  NMG+19,  NMG+53,  NMG+37,  NMG+69,  NMG+18, NMG+22,
    NMG-13,  NMG+4,   NMG+16,  NMG+13,  NMG+28,  NMG+19,  NMG+21, NMG-8,
    NMG-23,  NMG-9,   NMG+12,  NMG+10,  NMG+19,  NMG+17,  NMG+25, NMG-16,
    NMG-29,  NMG-53,  NMG-12,  NMG-3,   NMG-1,   NMG+18,  NMG-14, NMG-19,
    NMG-105, NMG-21,  NMG-58,  NMG-33,  NMG-17,  NMG-28,  NMG-19, NMG-23,
};

int eg_knight_table[64] = {
    NEG-58, NEG-38, NEG-13, NEG-28, NEG-31, NEG-27, NEG-63, NEG-99,
    NEG-25, NEG-8,  NEG-25, NEG-2,  NEG-9,  NEG-25, NEG-24, NEG-52,
    NEG-24, NEG-20, NEG+10, NEG+9,  NEG-1,  NEG-9,  NEG-19, NEG-41,
    NEG-17, NEG+3,  NEG+22, NEG+22, NEG+22, NEG+11, NEG+8,  NEG-18,
    NEG-18, NEG-6,  NEG+16, NEG+25, NEG+16, NEG+17, NEG+4,  NEG-18,
    NEG-23, NEG-3,  NEG-1,  NEG+15, NEG+10, NEG-3,  NEG-20, NEG-22,
    NEG-42, NEG-20, NEG-10, NEG-5,  NEG-2,  NEG-20, NEG-23, NEG-44,
    NEG-29, NEG-51, NEG-23, NEG-15, NEG-22, NEG-18, NEG-50, NEG-64,
};

int mg_bishop_table[64] = {
    BMG-29,  BMG+4,   BMG-82,  BMG-37,  BMG-25,  BMG-42,  BMG+7,   BMG-8,
    BMG-26,  BMG+16,  BMG-18,  BMG-13,  BMG+30,  BMG+59,  BMG+18,  BMG-47,
    BMG-16,  BMG+37,  BMG+43,  BMG+40,  BMG+35,  BMG+50,  BMG+37,  BMG-2,
    BMG-4,   BMG+5,   BMG+19,  BMG+50,  BMG+37,  BMG+37,  BMG+7,   BMG-2,
    BMG-6,   BMG+13,  BMG+13,  BMG+26,  BMG+34,  BMG+12,  BMG+10,  BMG+4,
    BMG+0,   BMG+15,  BMG+15,  BMG+15,  BMG+14,  BMG+27,  BMG+18,  BMG+10,
    BMG+4,   BMG+15,  BMG+16,  BMG+0,   BMG+7,   BMG+21,  BMG+33,  BMG+1,
    BMG-33,  BMG-3,   BMG-14,  BMG-21,  BMG-13,  BMG-12,  BMG-39,  BMG-21,
};

int eg_bishop_table[64] = {
    BEG-14, BEG-21, BEG-11, BEG-8,  BEG-7,  BEG-9,  BEG-17, BEG-24,
    BEG-8,  BEG-4,  BEG+7,  BEG-12, BEG-3,  BEG-13, BEG-4,  BEG-14,
    BEG+2,  BEG-8,  BEG+0,  BEG-1,  BEG-2,  BEG+6,  BEG+0,  BEG+4,
    BEG-3,  BEG+9,  BEG+12, BEG+9,  BEG+14, BEG+10, BEG+3,  BEG+2,
    BEG-6,  BEG+3,  BEG+13, BEG+19, BEG+7,  BEG+10, BEG-3,  BEG-9,
    BEG-12, BEG-3,  BEG+8,  BEG+10, BEG+13, BEG+3,  BEG-7,  BEG-15,
    BEG-14, BEG-18, BEG-7,  BEG-1,  BEG+4,  BEG-9,  BEG-15, BEG-27,
    BEG-23, BEG-9,  BEG-23, BEG-5,  BEG-9,  BEG-16, BEG-5,  BEG-17,
};

int mg_rook_table[64] = {
    RMG+32,  RMG+42,  RMG+32,  RMG+51,  RMG+63,  RMG+9,   RMG+31,  RMG+43,
    RMG+27,  RMG+32,  RMG+58,  RMG+62,  RMG+80,  RMG+67,  RMG+26,  RMG+44,
    RMG-5,   RMG+19,  RMG+26,  RMG+36,  RMG+17,  RMG+45,  RMG+61,  RMG+16,
    RMG-24,  RMG-11,  RMG+7,   RMG+26,  RMG+24,  RMG+35,  RMG-8,   RMG-20,
    RMG-36,  RMG-26,  RMG-12,  RMG-1,   RMG+9,   RMG-7,   RMG+6,   RMG-23,
    RMG-45,  RMG-25,  RMG-16,  RMG-17,  RMG+3,   RMG+0,   RMG-5,   RMG-33,
    RMG-44,  RMG-16,  RMG-20,  RMG-9,   RMG-1,   RMG+11,  RMG-6,   RMG-71,
    RMG-19,  RMG-13,  RMG+1,   RMG+17,  RMG+16,  RMG+7,   RMG-37,  RMG-26,
};

int eg_rook_table[64] = {
    REG+13,  REG+10,  REG+18,  REG+15,  REG+12,  REG+12,  REG+8,   REG+5,
    REG+11,  REG+13,  REG+13,  REG+11,  REG-3,   REG+3,   REG+8,   REG+3,
    REG+7,   REG+7,   REG+7,   REG+5,   REG+4,   REG-3,   REG-5,   REG-3,
    REG+4,   REG+3,   REG+13,  REG+1,   REG+2,   REG+1,   REG-1,   REG+2,
    REG+3,   REG+5,   REG+8,   REG+4,   REG-5,   REG-6,   REG-8,   REG-11,
    REG-4,   REG+0,   REG-5,   REG-1,   REG-7,   REG-12,  REG-8,   REG-16,
    REG-6,   REG-6,   REG+0,   REG+2,   REG-9,   REG-9,   REG-11,  REG-3,
    REG-9,   REG+2,   REG+3,   REG-1,   REG-5,   REG-13,  REG+4,   REG-20,
};

int mg_queen_table[64] = {
    QMG-28,  QMG+0,   QMG+29,  QMG+12,  QMG+59,  QMG+44,  QMG+43,  QMG+45,
    QMG-24,  QMG-39,  QMG-5,   QMG+1,   QMG-16,  QMG+57,  QMG+28,  QMG+54,
    QMG-13,  QMG-17,  QMG+7,   QMG+8,   QMG+29,  QMG+56,  QMG+47,  QMG+57,
    QMG-27,  QMG-27,  QMG-16,  QMG-16,  QMG-1,   QMG+17,  QMG-2,   QMG+1,
    QMG-9,   QMG-26,  QMG-9,   QMG-10,  QMG-2,   QMG-4,   QMG+3,   QMG-3,
    QMG-14,  QMG+2,   QMG-11,  QMG-2,   QMG-5,   QMG+2,   QMG+14,  QMG+5,
    QMG-35,  QMG-8,   QMG+11,  QMG+2,   QMG+8,   QMG+15,  QMG-3,   QMG+1,
    QMG-1,   QMG-18,  QMG-9,   QMG+10,  QMG-15,  QMG-25,  QMG-31,  QMG-50,
};

int eg_queen_table[64] = {
    QEG-9,   QEG+22,  QEG+22,  QEG+27,  QEG+27,  QEG+19,  QEG+10,  QEG+20,
    QEG-17,  QEG+20,  QEG+32,  QEG+41,  QEG+58,  QEG+25,  QEG+30,  QEG+0,
    QEG-20,  QEG+6,   QEG+9,   QEG+49,  QEG+47,  QEG+35,  QEG+19,  QEG+9,
    QEG+3,   QEG+22,  QEG+24,  QEG+45,  QEG+57,  QEG+40,  QEG+57,  QEG+36,
    QEG-18,  QEG+28,  QEG+19,  QEG+47,  QEG+31,  QEG+34,  QEG+39,  QEG+23,
    QEG-16,  QEG-27,  QEG+15,  QEG+6,   QEG+9,   QEG+17,  QEG+10,  QEG+5,
    QEG-22,  QEG-23,  QEG-30,  QEG-16,  QEG-16,  QEG-23,  QEG-36,  QEG-32,
    QEG-33,  QEG-28,  QEG-22,  QEG-43,  QEG-5,   QEG-32,  QEG-20,  QEG-41,
};

int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

int compute_phase() {
    const int KnightPhase = 1;
    const int BishopPhase = 1;
    const int RookPhase = 2;
    const int QueenPhase = 4;
    //const int TotalPhase = KnightPhase*4 + BishopPhase*4 + RookPhase*4 + QueenPhase*2;

    int game_phase = 0;

    game_phase += __builtin_popcountll(game.piece[wKnight]) * KnightPhase;
    game_phase += __builtin_popcountll(game.piece[wBishop]) * BishopPhase;
    game_phase += __builtin_popcountll(game.piece[wRook]) * RookPhase;
    game_phase += __builtin_popcountll(game.piece[wQueen]) * QueenPhase;

    game_phase += __builtin_popcountll(game.piece[bKnight]) * KnightPhase;
    game_phase += __builtin_popcountll(game.piece[bBishop]) * BishopPhase;
    game_phase += __builtin_popcountll(game.piece[bRook]) * RookPhase;
    game_phase += __builtin_popcountll(game.piece[bQueen]) * QueenPhase;

    return game_phase;
}
int mgPhase = 0;
int egPhase = 0;

// When doing material evaluation, we will do it inside the loop
// because popcount is very slow (when done this many times).

int eval_pawns() {
    int opening = 0;
    int endgame = 0;
    int sq;
    //BITBOARD mask = 0;

    BITBOARD bb = game.piece[wPawn];
    while (bb) {
        sq = __builtin_ctzll(bb);
        sq ^= 56;

        opening += mg_pawn_table[sq];
        endgame += eg_pawn_table[sq];

        bb &= bb - 1;
    }

    bb = game.piece[bPawn];
    while (bb) {
        sq = __builtin_ctzll(bb);

        opening -= mg_pawn_table[sq];
        endgame -= eg_pawn_table[sq];

        bb &= bb - 1;
    }

    return (opening * mgPhase + endgame * egPhase) / 24;
}

int eval_knights() {
    int opening = 0;
    int endgame = 0;
    int sq;

    BITBOARD bb = game.piece[wKnight];
    while (bb) {
        sq = __builtin_ctzll(bb);
        sq ^= 56;

        opening += mg_knight_table[sq];
        endgame += eg_knight_table[sq];
        
        bb &= bb - 1;
    }

    bb = game.piece[bKnight];
    while (bb) {
        sq = __builtin_ctzll(bb);

        opening -= mg_knight_table[sq];
        endgame -= eg_knight_table[sq];
        
        bb &= bb - 1;
    }

    return (opening * mgPhase + endgame * egPhase) / 24;
}

int eval_bishop() {
    int opening = 0;
    int endgame = 0;
    int sq;

    BITBOARD bb = game.piece[wBishop];
    while (bb) {
        sq = __builtin_ctzll(bb);
        sq ^= 56;

        opening += mg_bishop_table[sq];
        endgame += eg_bishop_table[sq];

        bb &= bb - 1;
    }

    bb = game.piece[bBishop];
    while (bb) {
        sq = __builtin_ctzll(bb);

        opening -= mg_bishop_table[sq];
        endgame -= eg_bishop_table[sq];

        bb &= bb - 1;
    }

    return (opening * mgPhase + endgame * egPhase) / 24;
}

int eval_rook() {
    int opening = 0;
    int endgame = 0;
    int sq;

    BITBOARD bb = game.piece[wRook];
    while (bb) {
        sq = __builtin_ctzll(bb);
        sq ^= 56;

        opening += mg_rook_table[sq];
        endgame += eg_rook_table[sq];

        bb &= bb - 1;
    }

    bb = game.piece[bRook];
    while (bb) {
        sq = __builtin_ctzll(bb);

        opening -= mg_rook_table[sq];
        endgame -= eg_rook_table[sq];

        bb &= bb - 1;
    }

    return (opening * mgPhase + endgame * egPhase) / 24;
}

int eval_queen() {
    int opening = 0;
    int endgame = 0;
    int sq;

    BITBOARD bb = game.piece[wQueen];
    while (bb) {
        sq = __builtin_ctzll(bb);
        sq ^= 56;

        opening += mg_queen_table[sq];
        endgame += eg_queen_table[sq];

        bb &= bb - 1;
    }

    bb = game.piece[bQueen];
    while (bb) {
        sq = __builtin_ctzll(bb);

        opening -= mg_queen_table[sq];
        endgame -= eg_queen_table[sq];

        bb &= bb - 1;
    }

    return (opening * mgPhase + endgame * egPhase) / 24;;
}

int eval_king() {
    // We know there are always 2 kings on the board
    // This means that:
    // 1. We don't need to do material eval
    // 2. We don't need to loop the bitboards
    
    int w_sq = __builtin_ctzll(game.piece[wKing]);
    int b_sq = __builtin_ctzll(game.piece[bKing]);

    w_sq ^= 56;
    int opening = mg_king_table[w_sq] - mg_king_table[b_sq];
    int endgame = eg_king_table[w_sq] - eg_king_table[b_sq];

    return (opening * mgPhase + endgame * egPhase) / 24;;
}


int evaluate() {
    int score = 0;
    mgPhase = compute_phase();
    mgPhase = MIN(mgPhase, 24);
    egPhase = 24 - mgPhase;

    score += eval_pawns();
    score += eval_knights();
    score += eval_bishop();
    score += eval_rook();
    score += eval_queen();
    score += eval_king();

    int perspective = game.isWhite ? 1 : -1;
    score *= perspective;

    return score;
}