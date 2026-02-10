#include "search.h"

int negamax(int depth, int alpha, int beta, int ply);
SMove root_moves[218];
int   root_guess[218];
int   root_nMoves;



/* ==================== INFO PRINTING ==================== */
void print_pv();



typedef struct SInfo {
    int depth;
    int score;
    U64 nodes;
    U64 nps;
    U64 time;
} SInfo;
SInfo info;

void print_info() {
    info.nps = ((info.nodes * 1000) / (info.time+1));

    printf("info depth %d score ", 
        info.depth);

    if (info.score > 90000000) { // Winning mating score
        int full = ((WIN-info.score-game.ply)+1) >> 1;
        printf("mate %d ", full);
    }
    else if (info.score < -90000000) { // Losing mating score
        int full = ((WIN+info.score-game.ply)+1) >> 1;
        printf("mate -%d ", full);
    }
    else {
        printf("cp %d ", info.score);
    }

    printf("nodes %llu nps %llu time %llu pv ", 
        info.nodes, info.nps, info.time);

    print_pv();

    printf("\n");
    fflush(stdout);
}


/* ==================== TIMING FUNCTIONS ==================== */



// Timer used for stopping when time is up
int timer(int limit_ms, bool action) {
    static clock_t start_time;
    static int limit;

    if (action == START) {

        // Start timer
        start_time = clock();
        limit = limit_ms;
        return 0;

    } else if (action == TIMENOW) {

        // Check if time exceeded
        clock_t now = clock();
        int elapsed_ms = (int)(((now - start_time) * 1000) / CLOCKS_PER_SEC);

        if (elapsed_ms >= limit) {
            return 1; // Time is up
        } else {
            return 0; // Still within limit
        }

    }

    return 0; // Default: not timed out
}

// Chronometer used for measuring time
U64 chronometer(bool action) {
    static clock_t start;

    if (action == START) {
        start = clock();
        return 0;
    }
    if (action == TIMENOW) {
        clock_t end = clock();
        return (U64)((end - start) * 1000 / CLOCKS_PER_SEC);
    }

    return 0;
}



/* ==================== SEARCH HEURISTICS ==================== */



// ========== PRINCIPAL VARIATION SEARCH ==========
SMove pv_table[MAX_DEPTH][MAX_DEPTH];
int pv_length[MAX_DEPTH];

void init_pv() {
    for (int i = 0; i < MAX_DEPTH; i++) {
        pv_length[i] = 0;
        for (int j = 0; j < MAX_DEPTH; j++) {
            pv_table[i][j].move = 0;
            pv_table[i][j].moved = EMPTY;
            pv_table[i][j].captured = EMPTY;
        }
    }
}

void update_pv(SMove move, int ply) {
    pv_table[ply][ply] = move;

    // Copy the PV from the next ply
    for (int i = ply + 1; i < pv_length[ply + 1] + ply + 1; i++) {
        pv_table[ply][i] = pv_table[ply + 1][i];
    }

    // Update PV length
    pv_length[ply] = pv_length[ply + 1] + 1;
}

void print_pv() {
    for (int i = 0; i < pv_length[0]; i++) {
        print_move(pv_table[0][i]);
        printf(" ");
    }
}

// ========== LATE MOVE REDUCTIONS ==========
inline int lmr_reduction(int depth, int move) {
    if (depth < 3 || move <= 0) return 0;

    double r = 0.99 + log((double)depth) * log((double)move) / 3.14;
    return (int)round(r);
}

// ========== BUTTERFLY HISTORY TABLE ==========
int history[2][64][64];

void init_history() {
    for (int color = 0; color < 2; color++) {
        for (int from = 0; from < 64; from++) {
            for (int to = 0; to < 64; to++) {
                history[color][from][to] = 0;
            }
        }
    }
}

int clamp(int v, int lo, int hi) {
    if (v < lo) return lo;
    else if (v > hi) return hi;
    else return v;
}

void update_history(int from, int to, int bonus) {
    int clamped = clamp(bonus, MIN_HISTORY, MAX_HISTORY);
    history[game.isWhite][from][to] 
    += clamped - history[game.isWhite][from][to] * abs(clamped) / MAX_HISTORY;
}



/* ==================== MOVE ORDERING ==================== */



// Values used for MVV-LVA sorting of captures
const int piece_values[13] = {1, 3, 3, 5, 9, 20, 1, 3, 3, 5, 9, 20, 0};

// Sorting moves in quiesence search
// It only uses MVV-LVA
void sort_moves(SMove moves[], int n) {
    // No sorting if there are less than 2 moves
    if (n < 2) return;
    
    // Estimating the value of every move
    int guess[n];
    for (int i = 0; i < n; i++) {
        guess[i]  = piece_values[moves[i].captured] - piece_values[moves[i].moved];
        guess[i] += (moves[i].move & PROMOTIONFLAG) ? 10 : 0;
    }

    // Insertion sort in descending order
    for (int i = 1; i < n; i++) {
        int s = guess[i];
        SMove m = moves[i];
        int j = i - 1;

        while (j >= 0 && guess[j] < s) {
            guess[j + 1] = guess[j];
            moves[j + 1] = moves[j];
            j--;
        }

        guess[j + 1] = s;
        moves[j + 1] = m;
    }
}


// Main move ordering functions
void score_captures(SMove moves[], int guess[], int n) {
    // No sorting if there are less than 2 moves
    if (n < 2) return;
    
    // Estimating the value of every capture / promotion
    for (int i = 0; i < n; i++) {
        guess[i]  = piece_values[moves[i].captured] - piece_values[moves[i].moved];
        guess[i] += (moves[i].move & PROMOTIONFLAG) ? 10 : 0;
    }
}

void score_quiets(SMove moves[], int guess[], int n) {
    // No sorting if there are less than 2 moves
    if (n < 2) return;
    
    // Estimating the value of every quiet move
    for (int i = 0; i < n; i++) {
        guess[i] = history[game.isWhite][FROM(moves[i].move)][TO(moves[i].move)];
    }
}

void pick_move(SMove moves[], int score[], int n, int i) {
    int max = score[i];  // Start with current move's score
    int best_index = i;
    
    // Find the index of the maximum score
    for (int index = i + 1; index < n; index++) {
        if (score[index] > max) {
            max = score[index];
            best_index = index;
        }
    }
    
    // If we found a better move, "insert" it at position i
    if (best_index != i) {
        SMove best_move = moves[best_index];
        int best_score = score[best_index];
        
        // Shift moves right to make room
        for (int j = best_index; j > i; j--) {
            moves[j] = moves[j - 1];
            score[j] = score[j - 1];
        }
        
        // Insert the best move at position i
        moves[i] = best_move;
        score[i] = best_score;
    }
}


// root_sort is called only in the root node
// Assumes we have already evaluated each move
void root_sort(SMove moves[], int guess[], int n) {
    // No sorting if there are less than 2 moves
    if (n < 2) return;

    for (int i = 1; i < n; i++) {
        int s = guess[i];
        SMove m = moves[i];
        int j = i - 1;

        while (j >= 0 && guess[j] < s) {
            guess[j + 1] = guess[j];
            moves[j + 1] = moves[j];
            j--;
        }

        guess[j + 1] = s;
        moves[j + 1] = m;
    }
}

// quick_guess is called only in the root node to
// evaluate the moves based on their static eval
void quick_guess(SMove moves[], int guess[], int n) {
    // No need for sorting if there is only one move
    if (n < 2) return;

    SState s;
    for (int i = 0; i < n; i++) {
        do_move(moves[i], &s);
        guess[i] = evaluate();
        undo_move(moves[i], &s);
    }
}



/* ==================== QUIESCENCE SEARCH ==================== */



// Quiesence search - code from the CPW
int quiesce(int alpha, int beta) {
    info.nodes++;

    // Quit the search when time is up at the beginning
    if (timer(0, TIMENOW)) return NULLSCORE;

    int eval = evaluate();
    int score = 0;

    // Stand pat
    if(eval >= beta) return eval;
    if(eval > alpha) alpha = eval;

    SMove moves[30]; 
    int nMoves = 0;
    gen_capture(moves, &nMoves);
    sort_moves(moves, nMoves);

    SState s;
    for (int i = 0; i < nMoves; i++)  {
        do_move(moves[i], &s);
        score = -quiesce(-beta, -alpha);
        undo_move(moves[i], &s);

        if(score >= beta) return score;
        if(score > eval) eval = score;
        if(score > alpha) alpha = score;
    }

    return eval;
}



/* ==================== SEARCH ROOTS ==================== */



int root_search(int depth, int alpha, int beta) {
    // Quit the search when time is up at the beginning
    if (timer(0, TIMENOW)) return NULLSCORE;

    // Check extension
    if (is_check()) depth++;

    int eval = root_guess[0];
    SState s;
    for (int i = 0; i < root_nMoves; i++) {
        do_move(root_moves[i], &s);
        eval = -negamax(depth-1, -beta, -alpha, 1);
        undo_move(root_moves[i], &s);

        if (eval == NULLSCORE) break;
        root_guess[i] = eval;

        if (eval >= beta) return beta;
        if (eval > alpha) {
            alpha = eval;

            //Update pv table
            update_pv(root_moves[i], 0);
        }
    }

    return alpha;
}

// Aspiration window - code from CPW-Engine
int search_window(int depth, int eval) {
    int temp = eval;
    // Search window of 50 centipawns
    int alpha = eval - 50;
    int beta  = eval + 50;

    // Try search window
    temp = root_search(depth, alpha, beta);
    if ((temp <= alpha) || (temp >= beta))
        // If it failed high or low, 
        // redo search without a window
        temp = root_search(depth, LOS, WIN);

    return temp;
}

SMove iterate_search(int time) {
    // Initialize root data

    init_pv();

    info.depth = 0;
    info.score = 0;
    info.nodes = 0;
    info.nps = 0;
    info.time = 0;

    SMove nullmove;
    nullmove.move     = 0;
    nullmove.moved    = EMPTY;
    nullmove.captured = EMPTY;

    for (int i = 0; i < 218; i++) {
        root_moves[i] = nullmove;
        root_guess[i] = LOS;
    }
    root_nMoves = 0;

    gen_capture(root_moves, &root_nMoves);
    gen_quiet(root_moves, &root_nMoves);

    quick_guess(root_moves, root_guess, root_nMoves);
    root_sort(root_moves, root_guess, root_nMoves);

    U64 garbage;
    int eval = root_guess[0];

    timer(time, START);
    garbage = chronometer(START);

    for (int depth = 1; depth < MAX_DEPTH; depth++) {
        eval = search_window(depth, eval);

        // Break if we exceed the time limit
        if (timer(0, TIMENOW)) break;
        root_sort(root_moves, root_guess, root_nMoves);

        info.time = chronometer(TIMENOW);
        info.depth = depth;
        info.score = eval;
        print_info();


        if ((root_nMoves == 1) && (depth == 6)) break; // Break early if there is only one move avalible
    }

    return root_moves[0];
}



// ============================================================== //
/* ==================== MAIN SEARCH FUNCTION ==================== */
// ============================================================== //



int negamax(int depth, int alpha, int beta, int ply) {
    // Quit the search when time is up at the beginning
    if (timer(0, TIMENOW)) return NULLSCORE;

    // Handle PV and PVS
    pv_length[ply] = 0;
    int is_pv = (beta - alpha) > 1;

    // ========== MATE DISTANCE PRUNING ==========
    int mate_value = WIN - game.ply;
    if (alpha < -mate_value) alpha = -mate_value;
    if (beta > (mate_value-1)) beta = mate_value-1;
    if (alpha >= beta) return alpha;


    // ========== CHECK EXTENSIONS ==========
    // Making sure we never enter quiesence 
    // search while in check
    if (is_check()) depth++;


    // ========== QUIESCENCE SEARCH ==========
    if (depth == 0) return quiesce(alpha, beta);
    info.nodes++;


    // ========== DRAWS ==========
    if (three_fold()) return 0;
    if (game.fifty == 100) return 0;


    // ========== PROBING THE TT ==========
    SMove bestmove;
    bestmove.move     = 0;
    bestmove.moved    = EMPTY;
    bestmove.captured = EMPTY;
    int flag = TT_ALPHA;
    int eval = tt_probe(depth, alpha, beta, &bestmove);
    if (eval != NULLSCORE) {
        if (!is_pv || ((eval >= alpha) && (eval <= beta)))
            return eval;
    }
    U16 hashmove = bestmove.move;

    // First of all, search the hash move
    // Most likely, it's going to cause a cutoff
    SState s;
    int no_hash = 1;
    int no_moves = 1;
    if (hashmove != 0) {
        no_hash = 0;
        no_moves = 0;
        do_move(bestmove, &s);

        // For the hash move we do a full window search
        eval = -negamax(depth-1, -beta, -alpha, ply+1);

        undo_move(bestmove, &s);

        if (eval > alpha) {
            flag = TT_EXACT;

            if (eval >= beta) {
                if (!(hashmove & CAPTUREFLAG)) {
                    int bonus = depth*depth + 1;
                    // Apply the bonus in the history table
                    update_history(FROM(hashmove), TO(hashmove), bonus);
                }

                tt_save(depth, beta, TT_BETA, bestmove);
                return beta;
            }

            alpha = eval;

            // Update the PV table
            update_pv(bestmove, ply);
        }
    }


    // ========== REVERSE FUTILITY PRUNING ==========
    int do_rfp = 1;
    if (depth > 3) do_rfp = 0; // Only do RFP at low depths
    if (is_check()) do_rfp = 0; // Skip RFP when we are in check
    if (is_pv) do_rfp = 0; // Skip RFP on PV nodes
    if ((bestmove.move == 0) || (bestmove.move & CAPTUREFLAG)) 
        do_rfp = 0; // Skip RFP if TT move is null or capture

    if (do_rfp) {
        eval = evaluate();
        int margin = 150 * depth;
        if (eval >= (beta + margin))
            return (eval + beta) >> 1;
    }


    // ========== MOVE GENERATION ==========
    SMove list[218];
    int scores[218];

    // Second, search captures
    int nMoves = 0;
    int no_captures = 1;
    gen_capture(list, &nMoves);
    score_captures(list, scores, nMoves);

    if (nMoves != 0) {
        no_moves = 0;
        no_captures = 0;

        for (int i = 0; i < nMoves; i++) {
            // Put the next best move at the current index
            pick_move(list, scores, nMoves, i);
            // Skip the hash move, we've searched it already
            if (list[i].move == hashmove) continue;
            // Calculate reduction for the move
            int R = lmr_reduction(depth, i);

            do_move(list[i], &s);

            if (no_hash && (i == 0))
                // For the first move we do a full window search
                eval = -negamax(depth-1 - R, -beta, -alpha, ply+1);
            else {
                // Later moves are searched with a null window
                eval = -negamax(depth-1 - R, -alpha-1, -alpha, ply+1);

                // If the move is within alpha-beta bounds, re-search it with a full window
                if (eval > alpha && eval < beta) 
                    eval = -negamax(depth-1, -beta, -alpha, ply+1);

                // If the move improves alpha, re-search it with full depth
                else if (R > 0 && eval > alpha) {
                    eval = -negamax(depth - 1, -alpha-1, -alpha, ply+1);
                    // The move is within alpha-beta bounds, re-search it with a full window
                    if (eval > alpha && eval < beta)
                        eval = -negamax(depth-1, -beta, -alpha, ply+1);
                }
            }

            undo_move(list[i], &s);

            if (eval > alpha) {
                flag = TT_EXACT;
                bestmove = list[i];

                if (eval >= beta) {
                    tt_save(depth, beta, TT_BETA, bestmove);
                    return beta;
                }

                alpha = eval;

                // Update the PV table
                update_pv(bestmove, ply);
            }
        }
    }


    // Third, overwrite captures with quiet moves
    nMoves = 0;
    gen_quiet(list, &nMoves);
    score_quiets(list, scores, nMoves);
    
    if (nMoves != 0) {
        no_moves = 0;

        for (int i = 0; i < nMoves; i++) {
            // Put the next best move at the current index
            pick_move(list, scores, nMoves, i);
            // Skip the hash move, we've searched it already
            if (list[i].move == hashmove) continue;
            // Calculate reduction for the move
            int R = lmr_reduction(depth, i);

            do_move(list[i], &s);

            if (no_captures && no_hash && (i == 0))
                // For the first move we do a full window search
                eval = -negamax(depth-1 - R, -beta, -alpha, ply+1);
            else {
                // Later moves are searched with a null window
                eval = -negamax(depth-1 - R, -alpha-1, -alpha, ply+1);

                // If the move is within alpha-beta bounds, re-search it with a full window
                if (eval > alpha && eval < beta) 
                    eval = -negamax(depth-1, -beta, -alpha, ply+1);

                // If the move improves alpha, re-search it with full depth
                else if (R > 0 && eval > alpha) {
                    eval = -negamax(depth - 1, -alpha-1, -alpha, ply+1);
                    // The move is within alpha-beta bounds, re-search it with a full window
                    if (eval > alpha && eval < beta)
                        eval = -negamax(depth-1, -beta, -alpha, ply+1);
                }
            }

            undo_move(list[i], &s);

            if (eval > alpha) {
                flag = TT_EXACT;
                bestmove = list[i];

                if (eval >= beta) {
                    int bonus = depth*depth + 1;
                    int penalty = (bonus * 3) >> 2;
                    penalty = -penalty;
                    // Apply the bonus in the history table
                    update_history(FROM(list[i].move), TO(list[i].move), bonus);

                    // Apply penalty to all other quiet moves
                    for (int j = i-1; j >= 0; j--) {
                        update_history(FROM(list[j].move), TO(list[j].move), penalty);
                    }
                

                    tt_save(depth, beta, TT_BETA, bestmove);
                    return beta;
            
                }

                alpha = eval;

                // Update the PV table
                update_pv(bestmove, ply);
            }
        }
    }


    // ========== END OF GAME ==========
    if (no_moves) {
        // Checkmate
        if (is_check()) return -mate_value;
        // or stalemate...
        else return 0;
    }

    // Quit the search when time is up at the end
    if (timer(0, TIMENOW)) return NULLSCORE;

    tt_save(depth, alpha, flag, bestmove);
    return alpha;
}