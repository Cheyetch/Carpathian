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

#include "com.h"

void command_init() {
    init_hashes();
    init_attacks();
    init_magics();
    init_tt();
    init_history();
    init_pv();
}

void command_uciok() {
    printf("id name Carpathian %s\n", VERSION_STRING);
    printf("id author Cheyetch\n");
    printf("uciok\n");
    fflush(stdout);
}

void command_isready() {
    printf("readyok\n");
    fflush(stdout);
}

void command_newgame() {
    clear_board();
    init_tt();
    init_history();
    init_pv();
}

void command_startpos() {
    load_startpos();
}

void command_fen(char fen[]) {
    load_fen(fen);
}

void command_print() {
    print_board();
}

void command_perft(int depth) {
    perf_test(depth);
}

void command_bestmove(int time) {
    SMove move = iterate_search(time);
    printf("bestmove "); 
    print_move(move); 
    printf("\n");
    fflush(stdout);
}

void uci_loop() {
    char line[2048];

    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\n")] = '\0';

        char *saveptr;
        char *token = strtok_r(line, " ", &saveptr);

        if (strcmp(token, "quit") == 0) 
            return;

        else if (strcmp(token, "uci") == 0) 
            command_uciok();

        else if (strcmp(token, "ucinewgame") == 0) 
            command_newgame();

        else if (strcmp(token, "isready") == 0) 
            command_isready();

        else if (strcmp(token, "print") == 0) 
            command_print();

        else if (strcmp(token, "position") == 0) {
            int m = 1;
            token = strtok_r(NULL, " ", &saveptr);

            if (strcmp(token, "startpos") == 0) {
                command_startpos();
                token = strtok_r(NULL, " ", &saveptr);
            }

            else if (strcmp(token, "fen") == 0) {
                command_fen(saveptr);
                for (int i = 0; i < 7; i++) {
                    token = strtok_r(NULL, " ", &saveptr);
                    if (token == NULL) break;
                    else if (strcmp(token, "moves") == 0) break;
                }
            }

            else printf("Unknown command \"%s\"\n", token);

            if (token == NULL) m = 0;
            if (m) {if (strcmp(token, "moves") != 0) m = 0;}

            if (m) token = strtok_r(NULL, " ", &saveptr);
            while (token && m) {
                read_move(token);
                token = strtok_r(NULL, " ", &saveptr);
            }
        }

        else if (strcmp(token, "go") == 0) {
            token = strtok_r(NULL, " ", &saveptr);

            if (strcmp(token, "perft") == 0) {
                token = strtok_r(NULL, " ", &saveptr);
                int depth = atoi(token);
                command_perft(depth);
            }

            else if (strcmp(token, "wtime") == 0) {
                token = strtok_r(NULL, " ", &saveptr);
                int wtime = atoi(token);
                token = strtok_r(NULL, " ", &saveptr); token = strtok_r(NULL, " ", &saveptr);
                int btime = atoi(token);
                token = strtok_r(NULL, " ", &saveptr); token = strtok_r(NULL, " ", &saveptr);
                int winc = atoi(token);
                token = strtok_r(NULL, " ", &saveptr); token = strtok_r(NULL, " ", &saveptr);
                int binc = atoi(token);

                int time;
                if (game.isWhite) time = (wtime / 20) + (winc >> 1);
                else time              = (btime / 20) + (binc >> 1);

                command_bestmove(time);
            }
            else if (strcmp(token, "movetime") == 0) {
                token = strtok_r(NULL, " ", &saveptr);
                int time = atoi(token);
                
                command_bestmove(time);
            }
        }

        else printf("Unknown command \"%s\"\n", token);
    }
}

int main() {
    command_init();
    command_newgame();
    uci_loop();

    return 0;
}