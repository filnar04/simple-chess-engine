#include "chess.c"
#include "fen.c"
#include "util.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
uint8_t board[256] = EMPTY_BOARD;
struct gamestate initState;
unsigned int maxDepth = 1;
unsigned long long nodes[10];
int count = 0;

unsigned long long captures[10], enpassants[10], castles[10], promotions[10];
uint8_t moreInfo = 0;

char symbols[] = { ' ', 'P', 'N', 'B', 'R', 'Q', 'K' };
void
printBoard (uint8_t *board)
{
    for (int r = 8; r >= 1; r--) {
        for (int c = 1; c <= 8; c++) {
            int8_t pos = WIDTH * (r + 1) + c;
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
perft (uint8_t *board, struct gamestate gameState, unsigned depth)
{
    struct move moves[256] = { 0 };
    unsigned moveCount = searchMoves (board, gameState, moves);
    int8_t promotionRank = WHITE_PROMOTION;
    if (gameState.turn == BLACK) promotionRank = BLACK_PROMOTION;
    if (moveCount != 0) {
        uint8_t nextBoard[BOARD_MEM_SIZE];
        struct gamestate nextGameState;
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

        for (int i = 0; i < moveCount; i++) {
            nextGameState = gameState;
            nextGameState.turn ^= COLOR_MASK;
#ifdef SANITY_CHECK
            char error = 0;
            if ((board[moves[i].end] & PIECE_TYPE) == KING) {

                int8_t king = (gameState.turn == WHITE) ? gameState.kingB
                                                        : gameState.kingW;
                printf ("ERROR: capturing king at %c%d (king pos: %c%d) from "
                        "%c%d\n",
                        files[moves[i].end], ranks[moves[i].end], files[king],
                        ranks[king], files[moves[i].start],
                        ranks[moves[i].start]);
                printf ("capturing piece: %x, turn: %c\n",
                        board[moves[i].start],
                        (gameState.turn == WHITE) ? 'w' : 'b');
                error = 1;
            }
            if (board[moves[i].end] & gameState.turn) {
                printf ("ERROR: capturing own piece at %c%d from "
                        "%c%d\n",
                        files[moves[i].end], ranks[moves[i].end],
                        files[moves[i].start], ranks[moves[i].start]);
                printf ("capturing piece: %x, turn: %c\n",
                        board[moves[i].start],
                        (gameState.turn == WHITE) ? 'w' : 'b');
                error = 1;
            }
            if (error) {
                exit (EXIT_FAILURE);
                return;
            }
#endif
            if (moreInfo) {
                if (board[moves[i].end]) captures[depth]++;
                if ((board[moves[i].start] & PIECE_TYPE) == PAWN) {
                    if (files[moves[i].end] != files[moves[i].start]
                        && board[moves[i].end] == 0)
                        enpassants[depth]++;
                } else if ((board[moves[i].start] & PIECE_TYPE) == KING) {
                    if (abs (files[moves[i].start] - files[moves[i].end]) > 1)
                        castles[depth]++;
                }
                if ((board[moves[i].start] & PIECE_TYPE) == PAWN
                    && ranks[moves[i].end] == promotionRank) {
                    promotions[depth]++;
                }
            }
            if (depth + 1 < maxDepth) {
                memcpy (nextBoard, board, BOARD_MEM_SIZE);
                makeMove (nextBoard, &nextGameState, gameState.turn, moves[i]);
                perft (nextBoard, nextGameState, depth + 1);
            }
        }
    }

    nodes[depth] += moveCount;
    return;
}

int
main (int argc, char *argv[])
{
    FILE *source = NULL;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'i':
                moreInfo = 1;
                break;

            case 'f':
                source = fopen (argv[i + 1], "r");
                if (source == NULL) {
                    fprintf (stderr, "Can't open %s: ", argv[i + 1]);
                    perror ("");
                    exit (EXIT_FAILURE);
                }
                i++;
                break;
            case 'h':
                printf ("Usage: %s [depth] [-f path] [-i] [-h]\n", argv[0]);
                puts (
                    "-i count captures, castles, en passants and promotions");
                puts ("-f load starting position in FEN format from file "
                      "(https://en.wikipedia.org/wiki/"
                      "Forsyth%E2%80%93Edwards_Notation)");
                puts ("-h display this message and exit");
                exit (EXIT_SUCCESS);
                break;

            default:
                fprintf (stderr, "Unknown option: %c", argv[i][1]);
            }
        } else if (isdigit (argv[i][0])) {
            maxDepth = strtoul (argv[i], NULL, 0);
        } else {
            fprintf (stderr, "Unknown option: %s\n", argv[i]);
            fprintf (stderr, "Usage: %s [depth] [-f path] [-i] [-h]\n",
                     argv[0]);
        }
    }
    if (source) {
        char *fen = NULL;
        size_t len = 0;
        getline (&fen, &len, source);
        loadPosition (fen, board, &initState);
        free (fen);
        fclose (source);
    } else {
        // hardcoding goes brrrr
        initState.turn = WHITE;
        initState.kingW = 25;
        initState.kingB = 95;
        initState.shortcastle = WHITE | BLACK;
        initState.longcastle = WHITE | BLACK;
        initState.enpasssant = 0;
        initState.halfmove = 0;

        memcpy (board, currentBoard, BOARD_MEM_SIZE);
    }
    perft (board, initState, 0);
    printf ("depth\t|   nodes   ");
    if (moreInfo) printf ("| captures | castles  |   e. p.  | promotions");
    putchar ('\n');
    for (int i = 0; i < maxDepth; i++) {
        if (moreInfo) {
            printf ("%d\t| %10llu|%10llu|%10llu|%10llu|  %10llu\n", i + 1,
                    nodes[i], captures[i] + enpassants[i], castles[i],
                    enpassants[i], promotions[i]);
        } else
            printf ("%d\t| %10llu\n", i + 1, nodes[i]);
    }
}
