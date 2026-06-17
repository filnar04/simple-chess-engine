#include "chess.h"
#include "eval.h"
#include "ui.h"
#include "util.h"
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

static uint8_t board[] = STARTING_POS;

struct position {
    uint64_t boardState[8];
    uint repetitions;
};

static struct position *positionList;
static uint posListLen = 128;
static uint uniqPosCount = 0;

static int
posEqual (struct position *a, struct position *b)
{
    for (int i = 0; i < 8; i++) {
        if (a->boardState[i] != b->boardState[i]) return 0;
    }
    return 1;
}

static int
addPosition (uint8_t *board)
{
    struct position p;
    for (int i = FIRST_SQUARE, j = 0; i < LAST_SQUARE;
         i += VERTICAL_STEP, j++) {
        p.boardState[j] = *(uint64_t *)(board + i);
    }
    for (uint i = 0; i < uniqPosCount; i++) {
        if (posEqual (&p, &positionList[i])) {
            positionList[i].repetitions += 1;
            return positionList[i].repetitions;
        }
    }
    if (uniqPosCount == posListLen) {
        positionList = realloc (positionList,
                                (posListLen + 128) * sizeof (struct position));
        posListLen += 128;
    }
    positionList[uniqPosCount++] = p;
    return 1;
}

static uint8_t playerColor;
static struct gamestate gameState;
static uint8_t validMoves[64] = { 0 };

static void
nextMove ()
{
    if (gameState.turn == playerColor) {
        uint8_t ok = 0;
        uint8_t selected = 0;
        while (ok == 0) {
            while (selected == 0) {
                bzero (validMoves, 64);
                selected = uiSelectPiece (board, gameState, validMoves);
                showBoard (board, validMoves, playerColor);
            }
            ok = uiMakeMove (board, &gameState, &selected, validMoves);
            bzero (validMoves, 64);
            if (!ok && selected) {
                int8_t check
                    = testCheck (board,
                                 gameState.turn == WHITE ? gameState.kingW
                                                         : gameState.kingB,
                                 gameState.turn);
                getMoveArray (board, gameState, selected, check, validMoves);
            }
            showBoard (board, validMoves, playerColor);
        }
    } else {
        struct move best = getBestMove (board, &gameState, 20);
        makeMove (board, &gameState, gameState.turn, best);
        showBoard (board, validMoves, playerColor);
    }
}

int
main (int argc, char **argv)
{
    positionList = malloc (128 * sizeof (struct position));
    if (time (0) % 2) // TODO: add color selection
        playerColor = WHITE;
    else
        playerColor = BLACK;

    gameState.turn = WHITE;
    gameState.enpasssant = 0;
    gameState.halfmove = 0;
    gameState.kingB = 95;
    gameState.kingW = 25;
    gameState.longcastle = WHITE | BLACK;
    gameState.shortcastle = WHITE | BLACK;

    uiInit (argc, argv);
    showBoard (board, validMoves, playerColor);
    struct move moveList[300];
    enum gameResult result;
    while (1) { // TODO: move UI to separate thread
        bzero (moveList, 300);
        uint moveNum = searchMoves (board, gameState, moveList);
        if (moveNum == 0) {
            uint8_t king
                = gameState.turn == WHITE ? gameState.kingW : gameState.kingB;
            if (testCheck (board, king, gameState.turn)) {
                result = gameState.turn == WHITE ? CHECKMATE_BLACK
                                                 : CHECKMATE_WHITE;
                break;
            } else {
                result = DRAW_STALEMATE;
                break;
            }
        }
        nextMove ();
        if (addPosition (board) == 3) { // draw by repetition
            result = DRAW_REPETITION;
            break;
        } else if (gameState.halfmove >= 100) {
            result = DRAW_50MOVE;
            break;
        }
        gameState.turn ^= COLOR_MASK;
    }
    uiEnd (result);

    free (positionList);
    exit (EXIT_SUCCESS);
}
