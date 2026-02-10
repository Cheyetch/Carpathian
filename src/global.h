#ifndef GLOBAL_H
#define GLOBAL_H

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* Definitions */

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define VERSION_STRING "1.5"

typedef uint64_t BITBOARD;
typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;
typedef int64_t S64;
typedef int32_t S32;
typedef int16_t S16;
typedef int8_t S8;

// Constants

#define WIN 100000000
#define LOS -100000000
#define NULLSCORE LOS*2

#define NULLSQUARE 64
#define EMPTY 12

#define STARTFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define FROM(x) (x) & 0x3F
#define TO(x) ((x) >> 6) & 0x3F
#define SPECIAL(x) ((x) >> 12) & 0xF
#define CAPTUREFLAG 0x4000 // This is to be used on the whole move, not only on the special flags
#define PROMOTIONFLAG 0x8000 // This is to be used on the whole move, not only on the special flags

#define board 0xffffffffffffffffULL

#define fileA 0x101010101010101ULL
#define fileB 0x202020202020202ULL
#define fileC 0x404040404040404ULL
#define fileD 0x808080808080808ULL
#define fileE 0x1010101010101010ULL
#define fileF 0x2020202020202020ULL
#define fileG 0x4040404040404040ULL
#define fileH 0x8080808080808080ULL

#define rank1 0xffULL
#define rank2 0xff00ULL
#define rank3 0xff0000ULL
#define rank4 0xff000000ULL
#define rank5 0xff00000000ULL
#define rank6 0xff0000000000ULL
#define rank7 0xff000000000000ULL
#define rank8 0xff00000000000000ULL

typedef enum ESquares {
    a1 = 0, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
} ESquares;

typedef enum ECastle {
    wk = 1, wq = 2, bk = 4, bq = 8
} ECastle;

typedef enum EPieces {
    wPawn = 0, wKnight, wBishop, wRook, wQueen, wKing,
    bPawn = 6, bKnight, bBishop, bRook, bQueen, bKing
} EPieces;

typedef enum EColor {
    black = 0, white = 1
} EColor;

typedef enum ESpecial {
    quiet = 0, push, kCastle, qCastle, capture, ep,
    nPromo = 8, bPromo, rPromo, qPromo,
    nPromoCapture, bPromoCapture, rPromoCapture, qPromoCapture
} ESpecial;

// Structures

typedef struct SState {
    // Struct used in making and unmaking irreversible changes
    U8 castling;
    U8 epTarget;
    U8 fifty;
} SState;

typedef struct SBoard {
    BITBOARD piece[12];
    BITBOARD color[2]; // 0 = black, 1 = white
    U8 square[64];
    bool isWhite; // 0 = black, 1 = white
    U8 castling; // 1 = wShort, 2 = wLong, 4 = bShort, 8 = bLong
    U8 epTarget;
    U16 ply; // Also tracks repetition table index
    U8 fifty; // Tracks fifty move rule in ply

    U64 hash;
    U64 repetitionTable[1024]; 
    // Search only in-between ply-1 and ply-fifty
} SBoard;
extern SBoard game; // Global board

typedef struct SMove {
    U16 move;
    // Keep tracked of the moved piece and captured piece
    // In case of ep, the captured piece is a pawn but will be restored on the ep square
    U8 moved;
    U8 captured;
} SMove;

typedef struct SZobrist {
    U64 pieceSquare[12][64];
    U64 isBlack;
    U64 castling[16];
    U64 epSquare[65]; // 0 for element NULLSQUARE
} SZobrist;
extern SZobrist zobrist;

typedef struct STTEntry {
    U64 hash;
    int val;
    SMove bestmove;
    U8 depth;
    U8 flag;
} STTEntry;

typedef struct SMagic {
    U64* ptr;  // Pointer to attack_table for each particular square
    U64 mask;  // Mask relevant squares of both lines (no outer squares)
    U64 magic; // Magic 64-bit number
    int shift; // Shift right
} SMagic;

#endif // GLOBAL_H