#include "gen.h"

int BBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

int RBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

const int BitTable[64] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
    51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
    26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
    58, 20, 37, 17, 36, 8
};

BITBOARD knightTbl[64];
BITBOARD kingTbl[64];
BITBOARD attack_table[110000];

SMagic bishopTbl[64];
SMagic rookTbl[64];

U64 bmask(int sq) {
  U64 result = 0ULL;
  int rk = sq/8, fl = sq%8, r, f;
  for(r=rk+1, f=fl+1; r<=6 && f<=6; r++, f++) result |= (1ULL << (f + r*8));
  for(r=rk+1, f=fl-1; r<=6 && f>=1; r++, f--) result |= (1ULL << (f + r*8));
  for(r=rk-1, f=fl+1; r>=1 && f<=6; r--, f++) result |= (1ULL << (f + r*8));
  for(r=rk-1, f=fl-1; r>=1 && f>=1; r--, f--) result |= (1ULL << (f + r*8));
  return result;
}

U64 rmask(int sq) {
  U64 result = 0ULL;
  int rk = sq/8, fl = sq%8, r, f;
  for(r = rk+1; r <= 6; r++) result |= (1ULL << (fl + r*8));
  for(r = rk-1; r >= 1; r--) result |= (1ULL << (fl + r*8));
  for(f = fl+1; f <= 6; f++) result |= (1ULL << (f + rk*8));
  for(f = fl-1; f >= 1; f--) result |= (1ULL << (f + rk*8));
  return result;
}

U64 random_U64_fewbits() {
    return rand64() & rand64() & rand64();
}

int pop_1st_bit(U64 *bb) {
    U64 b = *bb ^ (*bb - 1);
    unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);
    return BitTable[(fold * 0x783a9b23) >> 26];
}

U64 index_to_U64(int index, int bits, U64 m) {
    int i, j;
    U64 result = 0ULL;
    for(i = 0; i < bits; i++) {
        j = pop_1st_bit(&m);
        if(index & (1 << i)) result |= (1ULL << j);
    }
    return result;
}

U64 ratt(int sq, U64 block) {
    U64 result = 0ULL;
    int rk = sq/8, fl = sq%8, r, f;
    for(r = rk+1; r <= 7; r++) {
        result |= (1ULL << (fl + r*8));
        if(block & (1ULL << (fl + r*8))) break;
    }
    for(r = rk-1; r >= 0; r--) {
        result |= (1ULL << (fl + r*8));
        if(block & (1ULL << (fl + r*8))) break;
    }
    for(f = fl+1; f <= 7; f++) {
        result |= (1ULL << (f + rk*8));
        if(block & (1ULL << (f + rk*8))) break;
    }
    for(f = fl-1; f >= 0; f--) {
        result |= (1ULL << (f + rk*8));
        if(block & (1ULL << (f + rk*8))) break;
    }
    return result;
}

U64 batt(int sq, U64 block) {
    U64 result = 0ULL;
    int rk = sq/8, fl = sq%8, r, f;
    for(r = rk+1, f = fl+1; r <= 7 && f <= 7; r++, f++) {
        result |= (1ULL << (f + r*8));
        if(block & (1ULL << (f + r * 8))) break;
    }
    for(r = rk+1, f = fl-1; r <= 7 && f >= 0; r++, f--) {
        result |= (1ULL << (f + r*8));
        if(block & (1ULL << (f + r * 8))) break;
    }
    for(r = rk-1, f = fl+1; r >= 0 && f <= 7; r--, f++) {
        result |= (1ULL << (f + r*8));
        if(block & (1ULL << (f + r * 8))) break;
    }
    for(r = rk-1, f = fl-1; r >= 0 && f >= 0; r--, f--) {
        result |= (1ULL << (f + r*8));
        if(block & (1ULL << (f + r * 8))) break;
    }
    return result;
}

int transform(U64 b, U64 magic, int bits) {
    return (int)((b * magic) >> (64 - bits));
}

U64 find_magic(int sq, int m, int bishop) {
    U64 mask, b[4096], a[4096], used[4096], magic;
    int i, j, k, n, fail;

    mask = bishop? bmask(sq) : rmask(sq);
    n = __builtin_popcountll(mask);

    for(i = 0; i < (1 << n); i++) {
        b[i] = index_to_U64(i, n, mask);
        a[i] = bishop? batt(sq, b[i]) : ratt(sq, b[i]);
    }
    for(k = 0; k < 100000000; k++) {
        magic = random_U64_fewbits();
        if(__builtin_popcountll((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
        for(i = 0; i < 4096; i++) used[i] = 0ULL;
        for(i = 0, fail = 0; !fail && i < (1 << n); i++) {
        j = transform(b[i], magic, m);
        if(used[j] == 0ULL) used[j] = a[i];
        else if(used[j] != a[i]) fail = 1;
        }
        if(!fail) return magic;
    }
    printf("***Failed***\n");
    return 0ULL;
}

void init_attacks() {
    for (int i = 0; i < 64; i++) {
        BITBOARD b = 1ULL << i;
        BITBOARD attacks;

        // Knight attacks
        U64 l1 = (b >> 1) & 0x7f7f7f7f7f7f7f7fULL;
        U64 l2 = (b >> 2) & 0x3f3f3f3f3f3f3f3fULL;
        U64 r1 = (b << 1) & 0xfefefefefefefefeULL;
        U64 r2 = (b << 2) & 0xfcfcfcfcfcfcfcfcULL;
        U64 h1 = l1 | r1; // Used for UP UP LEFT/RIGHT
        U64 h2 = l2 | r2; // Used for UP LEFT/RIGHT LEFT/RIGHT
        attacks = (h1<<16) | (h1>>16) | (h2<<8) | (h2>>8);
        knightTbl[i] = attacks;

        // King attacks
        attacks = ((b << 1) & 0xfefefefefefefefeULL) | ((b >> 1) & 0x7f7f7f7f7f7f7f7fULL); // Left and right
        b          |= attacks; // This modifies b, but it doesn't matter anymore
        attacks    |= (b << 8) | (b >> 8); // Up and down
        kingTbl[i] = attacks;
    }
}

void init_magics() {
    U64* ptr = attack_table; // Start of the big table
    
    // Initialize rook magics
    for (int sq = 0; sq < 64; sq++) {
        rookTbl[sq].ptr = ptr;
        rookTbl[sq].mask = rmask(sq);
        rookTbl[sq].magic = find_magic(sq, RBits[sq], 0);
        rookTbl[sq].shift = 64 - RBits[sq];
        
        // Fill this square's section of the table
        int n = __builtin_popcountll(rookTbl[sq].mask);
        for (int i = 0; i < (1 << n); i++) {
            U64 occ = index_to_U64(i, n, rookTbl[sq].mask);
            int idx = (occ * rookTbl[sq].magic) >> rookTbl[sq].shift;
            ptr[idx] = ratt(sq, occ);
        }
        
        ptr += (1 << RBits[sq]);
    }
    
    // Initialize bishop magics
    for (int sq = 0; sq < 64; sq++) {
        bishopTbl[sq].ptr = ptr;
        bishopTbl[sq].mask = bmask(sq);
        bishopTbl[sq].magic = find_magic(sq, BBits[sq], 1);
        bishopTbl[sq].shift = 64 - BBits[sq];
        
        int n = __builtin_popcountll(bishopTbl[sq].mask);
        for (int i = 0; i < (1 << n); i++) {
            U64 occ = index_to_U64(i, n, bishopTbl[sq].mask);
            int idx = (occ * bishopTbl[sq].magic) >> bishopTbl[sq].shift;
            ptr[idx] = batt(sq, occ);
        }
        
        ptr += (1 << BBits[sq]);
    }
}

inline BITBOARD wpawnSinglePush(int sq) {
    return 1ULL << (8 + sq);
}
inline BITBOARD wpawnDoublePush(int sq) {
    return (1ULL << (16 + sq)) & 0xff000000ULL;
}
inline BITBOARD wpawnCaptures(int sq) {
    BITBOARD b = 1ULL << sq;
    BITBOARD attacks = 0ULL;
    attacks |= (b << 7) & 0x7f7f7f7f7f7f7f7fULL; // Capture left
    attacks |= (b << 9) & 0xfefefefefefefefeULL; // Capture right
    return attacks;
}
inline BITBOARD bpawnSinglePush(int sq) {
    return (1ULL << sq) >> 8;
}
inline BITBOARD bpawnDoublePush(int sq) {
    return ((1ULL << sq) >> 16) & 0xff00000000ULL;
}
inline BITBOARD bpawnCaptures(int sq) {
    BITBOARD b = 1ULL << sq;
    BITBOARD attacks = 0ULL;
    attacks |= (b >> 9) & 0x7f7f7f7f7f7f7f7fULL; // Capture left
    attacks |= (b >> 7) & 0xfefefefefefefefeULL; // Capture right
    return attacks;
}
inline BITBOARD knightAttacks(int sq) {return knightTbl[sq];}
inline BITBOARD kingAttacks(int sq) {return kingTbl[sq];}
inline BITBOARD bishopAttacks(U64 occ, int sq) {
    U64* aptr = bishopTbl[sq].ptr;
    occ      &= bishopTbl[sq].mask;
    occ      *= bishopTbl[sq].magic;
    occ     >>= bishopTbl[sq].shift;
    return aptr[occ];
}
inline BITBOARD rookAttacks(U64 occ, int sq) {
    U64* aptr = rookTbl[sq].ptr;
    occ      &= rookTbl[sq].mask;
    occ      *= rookTbl[sq].magic;
    occ     >>= rookTbl[sq].shift;
    return aptr[occ];
}

int is_attacked(int side, int sq, BITBOARD occ, BITBOARD not) {
    BITBOARD attacks, pieces;
    // Pawn attacks
    if (side) {
        attacks = bpawnCaptures(sq);
        pieces  = game.piece[wPawn] & ~(not);
        if (attacks & pieces) return 1;
    }
    else {
        attacks = wpawnCaptures(sq);
        pieces  = game.piece[bPawn] & ~(not);
        if (attacks & pieces) return 1;
    }

    // Knight attacks
    attacks = knightAttacks(sq);
    pieces  = game.piece[bKnight - side] & ~(not);
    if (attacks & pieces) return 1;

    // Bishop attacks
    attacks = bishopAttacks(occ, sq);
    pieces  = (game.piece[bBishop - side] | game.piece[bQueen - side]) & ~(not);
    if (attacks & pieces) return 1;

    // Rook attacks
    attacks = rookAttacks(occ, sq);
    pieces  = (game.piece[bRook - side] | game.piece[bQueen - side]) & ~(not);
    if (attacks & pieces) return 1;

    // King attacks
    attacks = kingAttacks(sq);
    pieces  = game.piece[bKing - side] & ~(not);
    if (attacks & pieces) return 1;

    return 0;
}

int is_check() {
    if (is_attacked(6 * (!game.isWhite), __builtin_ctzll(game.piece[wKing+(6 * (!game.isWhite))]), game.color[0]|game.color[1], 0)) return 1;
    return 0;
}

inline void add_move(SMove moves[], int *n, U8 from, U8 to, U8 special, U8 moved, U8 captured) {
    BITBOARD occ = game.color[black] | game.color[white];
    BITBOARD not = 1ULL << to;
    occ         |= 1ULL << to;
    occ         &= ~(1ULL << from);
    if (special == ep) {
        occ     &= ~(1ULL << (to + ((moved == wPawn) ? -8:8)));
        not     |= 1ULL << (to + ((moved == wPawn) ? -8:8));
    }

    int side     = 6 * (!game.isWhite);
    int sq       = (__builtin_ctzll(game.piece[wKing + side]));
    sq = (sq == from) ? to : sq;

    if (is_attacked(side, sq, occ, not)) return;

    if (captured == wKing) return;
    if (captured == bKing) return;

    moves[*n] = encode_move(from, to, special, moved, captured);
    (*n)++;
}

void gen_quiet(SMove moves[], int *n) {
    BITBOARD bb, attacks, occ;
    int side, from, to;
    side = 6 * (!game.isWhite);
    occ = game.color[black] | game.color[white];

    /* First we genrate castling and pawn pushes */

    // Castle
    if (side && (game.castling & bk) && !(occ & 0x6000000000000000ULL)
    && !(is_attacked(side, e8, occ, 0) | is_attacked(side, f8, occ, 0) | is_attacked(side, g8, occ, 0))) {
        moves[*n] = encode_move(e8, g8, kCastle, bKing, EMPTY);
        (*n)++;
    }
    if (side && (game.castling & bq) && !(occ & 0xe00000000000000ULL)
    && !(is_attacked(side, e8, occ, 0) | is_attacked(side, d8, occ, 0) | is_attacked(side, c8, occ, 0))) {
        moves[*n] = encode_move(e8, c8, qCastle, bKing, EMPTY);
        (*n)++;
    }
    if (!side && (game.castling & wk) && !(occ & 0x60ULL)
    && !(is_attacked(side, e1, occ, 0) | is_attacked(side, f1, occ, 0) | is_attacked(side, g1, occ, 0))) {
        moves[*n] = encode_move(e1, g1, kCastle, wKing, EMPTY);
        (*n)++;
    }
    if (!side && (game.castling & wq) && !(occ & 0xeULL)
    && !(is_attacked(side, e1, occ, 0) | is_attacked(side, d1, occ, 0) | is_attacked(side, c1, occ, 0))) {
        moves[*n] = encode_move(e1, c1, qCastle, wKing, EMPTY);
        (*n)++;
    }

    // Pawn push
    bb = game.piece[wPawn + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        if (side) attacks = bpawnDoublePush(from) & ~(occ | (occ >> 8));
        else attacks      = wpawnDoublePush(from) & ~(occ | (occ << 8));
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, push, wPawn + side, EMPTY);
            attacks   = 0;
        }
    }

    // Pawn moves
    bb = game.piece[wPawn + side] & ~(side ? rank2 : rank7);
    while (bb) {
        from     = __builtin_ctzll(bb);
        if (side) attacks = bpawnSinglePush(from);
        else attacks      = wpawnSinglePush(from);
        attacks &= ~occ;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, quiet, wPawn+side, EMPTY);
            attacks   = 0;
        }
    }

    // Knight moves
    bb = game.piece[wKnight + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        attacks  = knightAttacks(from);
        attacks &= ~occ;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, quiet, wKnight + side, EMPTY);
            attacks  &= attacks - 1;
        }
    }

    // Bishop moves
    bb = game.piece[wBishop + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        attacks  = bishopAttacks(occ, from);
        attacks &= ~occ;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, quiet, wBishop + side, EMPTY);
            attacks  &= attacks - 1;
        }
    }

    // Rook moves
    bb = game.piece[wRook + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        attacks  = rookAttacks(occ, from);
        attacks &= ~occ;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, quiet, wRook + side, EMPTY);
            attacks  &= attacks - 1;
        }
    }

    // Queen moves
    bb = game.piece[wQueen + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        attacks  = bishopAttacks(occ, from) | rookAttacks(occ, from);
        attacks &= ~occ;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, quiet, wQueen + side, EMPTY);
            attacks  &= attacks - 1;
        }
    }

    // King moves
    bb = game.piece[wKing + side];
    from     = __builtin_ctzll(bb);
    attacks  = kingAttacks(from);
    attacks &= ~occ;
    bb      &= bb - 1;
    while (attacks) {
        to        = __builtin_ctzll(attacks);
        add_move(moves, n, from, to, quiet, wKing + side, EMPTY);
        attacks  &= attacks - 1;
    }
}

void gen_capture(SMove moves[], int *n) {
    BITBOARD bb, attacks, occ, opp;
    int side, from, to;
    side = 6 * (!game.isWhite);
    occ = game.color[black] | game.color[white];
    opp = game.color[!game.isWhite];

    /* First we generate pawn promotions and EP */

    // Pawn promotion
    bb = game.piece[wPawn + side] & (side ? rank2 : rank7);
    while (bb) {
        from     = __builtin_ctzll(bb);
        if (side) attacks = bpawnSinglePush(from);
        else attacks      = wpawnSinglePush(from);
        attacks &= ~occ;
        bb      &= bb - 1;

        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, qPromo, wPawn + side, EMPTY);
            add_move(moves, n, from, to, rPromo, wPawn + side, EMPTY);
            add_move(moves, n, from, to, bPromo, wPawn + side, EMPTY);
            add_move(moves, n, from, to, nPromo, wPawn + side, EMPTY);
            attacks   = 0;
        }

        if (side) attacks = bpawnCaptures(from);
        else attacks      = wpawnCaptures(from);
        attacks &= opp;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, qPromoCapture, wPawn + side, game.square[to]);
            add_move(moves, n, from, to, rPromoCapture, wPawn + side, game.square[to]);
            add_move(moves, n, from, to, bPromoCapture, wPawn + side, game.square[to]);
            add_move(moves, n, from, to, nPromoCapture, wPawn + side, game.square[to]);
            attacks  &= attacks - 1;
        }
    }

    // Pawn ep
    if (game.epTarget != NULLSQUARE) {
        to = game.epTarget;
        if (side) attacks = wpawnCaptures(to) & game.piece[bPawn];
        else attacks      = bpawnCaptures(to) & game.piece[wPawn];
        while (attacks) {
            from        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, ep, wPawn + side, bPawn - side);
            attacks  &= attacks - 1;
        }
    }

    // Pawn captures
    bb = game.piece[wPawn + side] & ~(side ? rank2 : rank7);
    while (bb) {
        from     = __builtin_ctzll(bb);
        if (side) attacks = bpawnCaptures(from);
        else attacks      = wpawnCaptures(from);
        attacks &= opp;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, capture, wPawn + side, game.square[to]);
            attacks  &= attacks - 1;
        }
    }

    // Knight captures
    bb = game.piece[wKnight + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        attacks  = knightAttacks(from);
        attacks &= opp;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, capture, wKnight + side, game.square[to]);
            attacks  &= attacks - 1;
        }
    }

    // Bishop captures
    bb = game.piece[wBishop + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        attacks  = bishopAttacks(occ, from);
        attacks &= opp;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, capture, wBishop + side, game.square[to]);
            attacks  &= attacks - 1;
        }
    }

    // Rook captures
    bb = game.piece[wRook + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        attacks  = rookAttacks(occ, from);
        attacks &= opp;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, capture, wRook + side, game.square[to]);
            attacks  &= attacks - 1;
        }
    }

    // Queen captures
    bb = game.piece[wQueen + side];
    while (bb) {
        from     = __builtin_ctzll(bb);
        attacks  = bishopAttacks(occ, from) | rookAttacks(occ, from);
        attacks &= opp;
        bb      &= bb - 1;
        while (attacks) {
            to        = __builtin_ctzll(attacks);
            add_move(moves, n, from, to, capture, wQueen + side, game.square[to]);
            attacks  &= attacks - 1;
        }
    }

    // King captures
    bb = game.piece[wKing + side];
    from     = __builtin_ctzll(bb);
    attacks  = kingAttacks(from);
    attacks &= opp;
    bb      &= bb - 1;
    while (attacks) {
        to        = __builtin_ctzll(attacks);
        add_move(moves, n, from, to, capture, wKing + side, game.square[to]);
        attacks  &= attacks - 1;
    }
}

U64 perft(int depth) {
    SMove moves[218];
    SState s;
    int n = 0;
    U64 nodes = 0;

    if (depth == 0) return 1ULL;

    gen_capture(moves, &n);
    gen_quiet(moves, &n);
    if (depth == 1) return (U64) n;

    for (int i = 0; i < n; i++) {
        do_move(moves[i], &s);
        nodes += perft(depth-1);
        undo_move(moves[i], &s);
    }

    return nodes;
}

void perf_test(int depth) {
    SMove moves[218];
    SState s;
    int n = 0;
    U64 nodes = 0, temp;

    if (depth == 0) {printf("\nNodes searched: 1\n\n"); return;}

    gen_capture(moves, &n);
    gen_quiet(moves, &n);
    for (int i = 0; i < n; i++) {
        do_move(moves[i], &s);
        temp   = perft(depth-1);
        nodes += temp;
        undo_move(moves[i], &s);

        print_move(moves[i]);
        printf(": %llu\n", temp);
    }

    printf("\nNodes searched: %llu\n\n", nodes);
}