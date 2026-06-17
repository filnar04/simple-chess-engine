#include "chess.h"
#include "ui.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static const char *const colors[] = { "48;2;128;64;0", "48;2;192;128;96",
                               "48;2;0;128;64", "48;2;96;192;128" };
static const char *const pColors[] = { "1;38;2;16;16;16", "1;38;2;239;239;239" };
static char *nerdSym[]
    = { " \0\0\0\0", "󰡙", "󰡘", "󰡜", "󰡛", "󰡚", "󰡗" };
static char *unicodeSym[] = { " \0", "♟", "♞", "♝", "♜", "♛", "♚" };
static char *asciiSym[] = { " ", "P", "N", "B", "R", "Q", "K" };
static char **symbols;

int
uiInit (int argc, char **argv)
{
    symbols = unicodeSym;
    for (int i = 0; i < argc; i++) {
        if (strncmp (argv[i], "--nerd", 6) == 0)
            symbols = nerdSym;
        else if (strncmp (argv[i], "--ascii", 7) == 0)
            symbols = asciiSym;
    }
    return 0;
}

void
showBoard (uint8_t *board, uint8_t *highlight, uint8_t side)
{
    puts ("\033[2J");
    int step, first, end;
    if (side == BLACK) {
        step = 1;
        first = 0;
        end = 8;
    } else {
        step = -1;
        first = 7;
        end = -1;
    }
    puts ("   a  b  c  d  e  f  g  h");
    for (int r = first; r != end; r += step) {
        printf ("\033[0m%d ", r + 1);
        for (int f = 0; f < 8; f++) {
            int8_t square = 8 * r + f;
            int8_t piece = board[BOARD_TO_LOGIC (square)];
            int8_t c_back = (f + r) % 2 + 2 * highlight[square];
            printf ("\033[%s;%sm %s ", colors[c_back],
                    pColors[(piece & COLOR_MASK) == WHITE],
                    symbols[piece & PIECE_TYPE]);
        }
        putchar ('\n');
    }
    puts ("\033[0m");
}

uint8_t
uiSelectPiece (uint8_t *board, struct gamestate gameState, uint8_t moves[64])
{
    char input[10];
    scanf ("%s", input);
    uint8_t playerColor = gameState.turn;
    int8_t square = LOGICAL_FROM_STR (input);
    if (!((board[square] & playerColor) && (board[square] & PIECE_TYPE))) {
        puts ("No piece here.");
        return 0;
    }
    int8_t kingPos
        = (playerColor == WHITE) ? gameState.kingW : gameState.kingB;
    char check = testCheck (board, kingPos, playerColor);
    if (getMoveArray (board, gameState, square, check, moves) == 0) {
        puts ("This piece has no moves.");
        return 0;
    }
    return square;
}

uint8_t
uiMakeMove (uint8_t *board, struct gamestate *gameState, uint8_t *start,
            uint8_t moveArr[64])
{
    int8_t dest;
    char input[20];
    uint8_t playerColor = gameState->turn;
    scanf ("%s", input);
    dest = LOGICAL_FROM_STR (input);
    if (!moveArr[LOGIC_TO_BOARD (dest)]) {
        if (board[dest] & gameState->turn)
            *start = dest;
        else
            *start = 0;
        return 0;
    }
    struct move mv;
    mv.start = *start;
    mv.end = dest;
    int8_t promRank
        = (playerColor == WHITE) ? WHITE_PROMOTION : BLACK_PROMOTION;
    int8_t p = 0;
    if (ranks[dest] == promRank && (board[*start] & PIECE_TYPE) == PAWN) {
        scanf ("%s", input);
        switch (input[0] | 32) {
        case 'n':
            p = KNIGHT;
            break;
        case 'b':
            p = BISHOP;
            break;
        case 'r':
            p = ROOK;
            break;
        default:
            p = QUEEN;
        }
        mv.promotion = p;
    } else {
        mv.promotion = 0;
    }
    makeMove (board, gameState, playerColor, mv);
    return 1;
}

void
uiEnd (enum gameResult result)
{
    switch (result) {
    case CHECKMATE_BLACK:
        puts ("Black won by checkmate.");
        break;
    case CHECKMATE_WHITE:
        puts ("White won by checkamte.");
        break;
    case DRAW_STALEMATE:
        puts ("The game ended in draw by stalemate.");
        break;
    case DRAW_50MOVE:
        puts ("The game ended in draw by 50 move rule.");
        break;
    case DRAW_REPETITION:
        puts ("The game ended in draw by repetition.");
        break;
    case DRAW_DEAD:
        puts ("The game ended in draw by insufficient material.");
        break;
    }
}
