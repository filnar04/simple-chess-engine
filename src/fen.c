#include "util.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

void
loadPosition (char *fen, uint8_t *board, struct gamestate *gameState)
{
    int8_t x = 0;
    int8_t square;
    int8_t rank = 8, file = 1;
    char kingW = 0, kingB = 0;
    while (fen[x] != ' ') {
        if (fen[x] == '/') {
            rank -= 1;
            file = 0;
        } else if (fen[x] >= '1' && fen[x] <= '8') {
            file += (fen[x] - '1');
        } else {
            square = VERTICAL_STEP * (rank + 1) + file;
            switch (fen[x]) {
            case 'K': {
                if (!kingW) {
                    board[square] = WHITE | KING;
                    gameState->kingW = square;
                    kingW = 1;
                } else
                    puts ("ERROR: another white king in position\n");
                break;
            }
            case 'k': {
                if (!kingB) {
                    board[square] = BLACK | KING;
                    gameState->kingB = square;
                    kingB = 1;
                } else
                    puts ("ERROR: another black king in position\n");
                break;
            }
            case 'Q':
                board[square] = WHITE | QUEEN;
                break;
            case 'q':
                board[square] = BLACK | QUEEN;
                break;
            case 'R':
                board[square] = WHITE | ROOK;
                break;
            case 'r':
                board[square] = BLACK | ROOK;
                break;
            case 'B':
                board[square] = WHITE | BISHOP;
                break;
            case 'b':
                board[square] = BLACK | BISHOP;
                break;
            case 'N':
                board[square] = WHITE | KNIGHT;
                break;
            case 'n':
                board[square] = BLACK | KNIGHT;
                break;
            case 'P':
                board[square] = WHITE | PAWN;
                if (ranks[square] != WHITE_PAWN_START)
                    board[square] |= HAS_MOVED;
                break;
            case 'p':
                board[square] = BLACK | PAWN;
                if (ranks[square] != BLACK_PAWN_START)
                    board[square] |= HAS_MOVED;
                break;
            default: {
                err (EXIT_FAILURE,
                     "invalid symbol in FEN string: '%c' at:%d (code:\\%d)\n",
                     fen[x], x, fen[x]);
                return;
            }
            }
        }
        file++;
        x++;
    }
    if (!kingW) {
        err (EXIT_FAILURE, "Loading position without white king\n");
    }
    if (!kingB) {
        err (EXIT_FAILURE, "Loading position without black king\n");
    }
    if (fen[++x] == 'b')
        gameState->turn = BLACK;
    else
        gameState->turn = WHITE;
    x += 1;
    while (fen[++x] != ' ') {
        switch (fen[x]) {
        case 'K': {
            gameState->shortcastle |= WHITE;
            break;
        }
        case 'Q': {
            gameState->longcastle |= WHITE;
            break;
        }
        case 'k': {
            gameState->shortcastle |= BLACK;
            break;
        }
        case 'q': {
            gameState->longcastle |= BLACK;
            break;
        }
        default:
            break;
        }
    }
    if (fen[x] != '-') {
        file = fen[x] - 'a' + 1;
        rank = fen[++x];
        gameState->enpasssant = VERTICAL_STEP * (rank - '0' + 1) + file;
    } else {
        gameState->enpasssant = 0;
    }
}
