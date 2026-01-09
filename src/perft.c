#include "chess.c"
#include "fen.c"
#include "util.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int8 boards[BOARD_MEM_SIZE * 256] = EMPTY_BOARD;
struct gamestate initState;
unsigned int maxDepth = 0;
unsigned long long nodes[10];
int count = 0;

unsigned long long captures[10], enpassants[10], castles[10], promotions[10];

char symbols[] = { ' ', 'P', 'N', 'B', 'R', 'Q', 'K' };
void
printBoard (int8 *board)
{
    for (int r = 8; r >= 1; r--) {
        for (int c = 1; c <= 8; c++) {
            int8 pos = WIDTH * (r + 1) + c;
            if ((r + c) & 1) {
                printf ("\033[103m");
            } else {
                printf ("\033[102m");
            }
            char s = symbols[board[pos] & PIECE_TYPE];
            if ((board[pos] & PIECE_TYPE) == 0 && board[pos] != 0) s = '.';
            if (board[pos] & BLACK) {
                printf ("\033[31m");
                s += 32;
            } else {
                printf ("\033[97m");
            }
            if ((board[pos] & PIECE_TYPE) == 0 && board[pos]) s = '.';
            putchar (s);
        }
        putchar ('\n');
    }
    puts ("\033[0m");
}

void
perft (int8 *board, struct gamestate gameState, int8 color, unsigned depth)
{
    int8 *l = calloc (256, 1);
    int8 *p = calloc (256, 1);
    unsigned moveCount = searchMoves (board, gameState, color, l, p);
    int8 promotionRank = WHITE_PROMOTION;
    if (color == BLACK) promotionRank = BLACK_PROMOTION;
    if (moveCount != 0) {
        int8 *nextBoard = board + BOARD_MEM_SIZE;
        struct gamestate nextGameState;
        memcpy (&nextGameState, &gameState, sizeof (struct gamestate));
        int8 start = 0;

        for (int i = 0; i < moveCount; i++) {

            memcpy (&nextGameState, &gameState, sizeof (struct gamestate));
            int8 promotion = 0;
            if (p[i] >= FIRST_SQUARE) {
                start = p[i];
            }
            int8 dest = l[i];
#ifdef DEBUG_INFO
#ifdef VERBOSE_DEBUG_INFO
            for (int j = 0; j < moveCount; j++) {
                if (p[j] >= FIRST_SQUARE)
                    printf ("%c%d ", files[p[j]], ranks[p[j]]);
                else
                    printf ("-- ");
            }
            putchar ('\n');
            for (int j = 0; j < moveCount; j++) {
                printf ("%c%d ", files[l[j]], ranks[l[j]]);
            }
            putchar ('\n');
#endif
            char error = 0;
            if (board[dest]) captures[depth]++;
            if ((board[dest] & PIECE_TYPE) == KING) {

                int8 king
                    = (color == WHITE) ? gameState.kingB : gameState.kingW;
                printf ("ERROR: capturing king at %c%d (king pos: %c%d) from "
                        "%c%d\n",
                        files[dest], ranks[dest], files[king], ranks[king],
                        files[start], ranks[start]);
                printf ("capturing piece: %x, turn: %c\n", board[start],
                        (color == WHITE) ? 'w' : 'b');
                error = 1;
            }
            if (board[dest] & color) {
                printf ("ERROR: capturing own piece at %c%d from "
                        "%c%d\n",
                        files[dest], ranks[dest], files[start], ranks[start]);
                printf ("capturing piece: %x, turn: %c\n", board[start],
                        (color == WHITE) ? 'w' : 'b');
                error = 1;
            }
            if (error) {
                for (int i = 0; i <= depth; i++) {
                    printBoard (board - BOARD_MEM_SIZE * i);
                }
                exit (EXIT_FAILURE);
                return;
            }
#endif
            if ((board[start] & PIECE_TYPE) == PAWN) {
                if (files[dest] != files[start] && board[dest] == 0)
                    enpassants[depth]++;
            } else if ((board[start] & PIECE_TYPE) == KING) {
                if (abs (files[start] - files[dest]) > 1) castles[depth]++;
            }
            if ((board[start] & PIECE_TYPE) == PAWN
                && ranks[l[i]] == promotionRank) {
                promotions[depth]++;
                if (p[i] > 0 && p[i] < FIRST_SQUARE)
                    promotion = p[i];
                else
                    promotion = QUEEN;
            }
            if (depth + 1 < maxDepth) {
                memcpy (nextBoard, board, BOARD_MEM_SIZE);
                makeMove (nextBoard, &nextGameState, start, l[i], promotion);
                perft (nextBoard, nextGameState, color ^ COLOR_MASK,
                       depth + 1);
            }
        }
    }

    free (l);
    free (p);
    nodes[depth] += moveCount;
    return;
}

int
main (int argc, char *argv[])
{
    if (argc < 2) return 0;
    if (argc >= 3) {
        FILE *source = fopen (argv[2], "r");
        char *fen = NULL;
        size_t len = 0;
        getline (&fen, &len, source);
        loadPosition (fen, boards, &initState);
        free (fen);
        printBoard (boards);
    } else {
        // hardcoding goes brrrr
        initState.turn = WHITE;
        initState.kingW = 25;
        initState.kingB = 95;
        initState.shortcastle = WHITE | BLACK;
        initState.longcastle = WHITE | BLACK;
        initState.enpasssant = 0;
        initState.halfmove = 0;

        memcpy (boards, currentBoard, BOARD_MEM_SIZE);
    }
    maxDepth = atoi (argv[1]);
    perft (boards, initState, initState.turn, 0);
    printf (
        "depth\t|   nodes   | captures | castles  |   e. p.  | promotions\n");
    for (int i = 0; i < maxDepth; i++) {
        printf ("%d\t| %10llu|%10llu|%10llu|%10llu|%10llu\n", i + 1, nodes[i],
                captures[i] + enpassants[i], castles[i], enpassants[i],
                promotions[i]);
    }
}
