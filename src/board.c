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

#include "board.h"

SBoard game;
SZobrist zobrist;
char characters[13] = "PNBRQKpnbrqk ";

U64 rand64() {
    // xorshift64star
    static U64 x = 1ULL;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

void init_hashes() {
    // Initialize piece hashes
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 64; j++) {
            zobrist.pieceSquare[i][j] = rand64();
        }
    }

    // Initializes ep hashes
    for (int i = 0; i < 64; i++) {
        zobrist.epSquare[i] = rand64();
    }
    zobrist.epSquare[64] = 0; // NULLSQUARE has a hash of 0

    // Initialize castling hashes
    for (int i = 0; i < 16; i++) {
        zobrist.castling[i] = rand64();
    }

    // Initialize the black to move hash
    zobrist.isBlack = rand64();
}

void update_hash() {
    // Must be called twice
    // The first time we exclude these out of the hash
    // The second time we include these back into the hash, after they are updated
    game.hash ^= zobrist.castling[game.castling];
    game.hash ^= zobrist.epSquare[game.epTarget];
}

void fill_square(U8 sq, U8 piece) {
    if (piece == EMPTY) return;

    BITBOARD bitSq = 1ULL << sq;
    game.piece[piece]   |= bitSq;
    game.square[sq]      = piece;
    game.color[piece<6] |= bitSq; // piece<6 = white
    game.hash           ^= zobrist.pieceSquare[piece][sq];
}

void clear_square(U8 sq) {
    BITBOARD bitSq = 1ULL << sq;
    U8 piece = game.square[sq];
    if (piece == EMPTY) return;

    game.piece[piece]   ^= bitSq;
    game.square[sq]      = EMPTY;
    game.color[piece<6] ^= bitSq; // piece<6 = white
    game.hash           ^= zobrist.pieceSquare[piece][sq];
}

void clear_board() {
    // Clears bitboards
    for (int i = 0; i < 12; i++) {
        game.piece[i] = 0;
    }

    // Clears board
    for (int i = 0; i < 64; i++) {
        game.square[i] = EMPTY;
    }

    // Clears repetition table
    for (int i = 0; i < 1024; i++) {
        game.repetitionTable[i] = 0;
    }

    game.color[0] = 0;
    game.color[1] = 0;
    game.isWhite = true;
    game.castling = 0;
    game.ply = 0;
    game.fifty = 0;
    game.epTarget = NULLSQUARE;
    game.hash = 0;
}

void load_fen(char fen[]) {
    // We have to load these fields in order
    // board -> side moving -> castling -> ep target -> half move clock -> full moves / converted to ply
    // We also update the initial hash in here

    // Sometimes, the last 2 fields are missing, assume 0 and 0 ply

    clear_board();
    char temp;
    int n = 0;

    // Field 1 - board setup
    
    int row = 7;
    int col = -1;
    int sq;

    temp = fen[n];
    while (temp != ' ') {
        col++;
        if (temp < '9') col += temp - '1';
        if (temp == '/') {row--; col = -1;}

        sq = row * 8 + col;
        switch (temp) {
            case 'P': fill_square(sq, wPawn);   break;
            case 'N': fill_square(sq, wKnight); break;
            case 'B': fill_square(sq, wBishop); break;
            case 'R': fill_square(sq, wRook);   break;
            case 'Q': fill_square(sq, wQueen);  break;
            case 'K': fill_square(sq, wKing);   break;

            case 'p': fill_square(sq, bPawn);   break;
            case 'n': fill_square(sq, bKnight); break;
            case 'b': fill_square(sq, bBishop); break;
            case 'r': fill_square(sq, bRook);   break;
            case 'q': fill_square(sq, bQueen);  break;
            case 'k': fill_square(sq, bKing);   break;

            default: break;
        }

        n++;
        temp = fen[n];
    }

    n++;

    // Field 2 - side moving

    temp = fen[n];
    if (temp == 'w') game.isWhite = true;
    else{game.isWhite = false; game.hash ^= zobrist.isBlack;}


    n+=2;

    // Field 3 - castling rights

    temp = fen[n];
    while (temp != ' ') {
        // Castling is initialized as 0, so we don't need to check for '-'
        if (temp == 'K') game.castling += wk;
        else if (temp == 'Q') game.castling += wq;
        else if (temp == 'k') game.castling += bk;
        else if (temp == 'q') game.castling += bq;

        n++;
        temp = fen[n];
    }

    n++;

    // Field 4 - ep square

    temp = fen[n];
    if (temp != '-') {
        col = temp - 'a';
        n++; temp = fen[n];
        row = temp - '1';

        sq = row * 8 + col;
        game.epTarget = sq;
    }
    else game.epTarget = NULLSQUARE;

    // Check if field 5 exists
    n++; temp = fen[n];
    if (temp == '\0') {
        game.fifty = 0;
        game.ply = 0;
        update_hash();
        return;
    }

    n++;

    // Field 5 - half move clock

    temp = fen[n];
    if (temp == 'm') {
        game.fifty = 0;
        game.ply = 0;
        update_hash();
        return;
    }
    
    while (temp != ' ') {
        game.fifty = game.fifty * 10 + temp - '0';

        n++;
        temp = fen[n];
    }

    n++;

    // Field 6 - full move counter

    temp = fen[n];
    int counter = 0;
    while ((temp != '\0') && (temp != ' ')) {
        counter = counter * 10 + temp - '0';

        n++;
        temp = fen[n];
    }
    game.ply = (counter - 1) * 2 + (!game.isWhite);

    update_hash();
}

void load_startpos() {
    load_fen(STARTFEN);
}

int three_fold() {
    U64 hash = game.hash;
    int r = 1;

    for (int i = game.ply-1; i > game.ply-game.fifty-1; i--) {
        if (game.repetitionTable[i] == hash) r++;
    }
    
    if (r >= 3) return 1;
    return 0;
}

void print_board() {
    for (int i = 7; i >= 0; i--) {
        printf("  +---+---+---+---+---+---+---+---+\n");
        printf("%d |", i+1);

        for (int j = 0; j < 8; j++) {
            printf (" %c |", characters[game.square[(i * 8) + j]]);
        }

        if (i == 7)      printf("   White moving:      %d", game.isWhite);
        else if (i == 6) printf("   Castling rights:   %d", game.castling);
        else if (i == 5) printf("   Ep square target:  %d", game.epTarget);
        else if (i == 4) printf("   Fifty move clock:  %d", game.fifty);
        else if (i == 3) printf("   Ply counter:       %d", game.ply);
        else if (i == 2) printf("   Position hash:     %llu", game.hash);

        printf("\n");
    }

    printf("  +---+---+---+---+---+---+---+---+\n");
    printf("    a   b   c   d   e   f   g   h  \n");
}

const char ASquares[64][3] = {
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

const char APromo[5] = "nbrq";

void print_move(SMove move) {
    int from = FROM(move.move);
    int to   = TO(move.move);
    printf("%s%s", ASquares[from], ASquares[to]);
    
    if (move.move & PROMOTIONFLAG) 
        printf("%c", APromo[SPECIAL(move.move) & 3]);
}