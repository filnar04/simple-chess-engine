#include "eval.h"
#include "chess.h"
#include "evalconsts.h"
#include "util.h"
#include <string.h>
#include <time.h>

int
eval (uint8_t *board, struct gamestate *state)
{
    if (state->halfmove >= 100) return 0;
    static const int16_t pieceVal[] = { 0, 100, 310, 330, 500, 900, 0 };
    int8_t pawnsOnFileB[8] = { 0 };
    int8_t pawnsOnFileW[8] = { 0 };
    int8_t rooksOnFileB[8] = { 0 };
    int8_t rooksOnFileW[8] = { 0 };
    int val[2] = { 0 };

    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            int sq = FIRST_SQUARE + VERTICAL_STEP * r + c;
            if (board[sq] & PIECE_TYPE) {
                int p = pieceVal[board[sq] & PIECE_TYPE];
                int8_t isBlack = (board[sq] & BLACK) > 0;
                val[isBlack] += p;
                switch (board[sq] & PIECE_TYPE) {
                case KNIGHT:
                    val[isBlack] += knightbonus[sq];
                    break;
                case PAWN:
                    if (isBlack) {
                        if (pawnsOnFileB[c]) val[1] += DOUBLE_PAWN_MOD;
                        pawnsOnFileB[c]++;
                        val[1] += (7 - r) * 3;
                    } else {
                        if (pawnsOnFileW[c]) val[0] += DOUBLE_PAWN_MOD;
                        pawnsOnFileW[c]++;
                        val[0] += (r) * 3;
                    }
                    break;

                case ROOK:
                    if (isBlack)
                        rooksOnFileB[c]++;
                    else
                        rooksOnFileW[c]++;
                    break;

                case BISHOP: { // negative points for bishops acting like pawns
                    int8_t front = VERTICAL_STEP;
                    if (isBlack) front = -VERTICAL_STEP;
                    if ((board[sq + front + 1] & PIECE_TYPE) == PAWN)
                        val[isBlack] += BLOCKED_BISHOP_MOD;
                    if ((board[sq + front - 1] & PIECE_TYPE) == PAWN)
                        val[isBlack] += BLOCKED_BISHOP_MOD;
                } break;
                case QUEEN:
                case KING:
                    break;
                default:
                    __builtin_unreachable ();
                }
            }
        }
    for (int i = 0; i < 8; i++) {
        if (rooksOnFileB[i]) {
            if (pawnsOnFileB[i] == 0) {
                val[1] += SEMI_OPEN_ROOK_MOD * rooksOnFileB[i];
                if (pawnsOnFileW[i] == 0)
                    val[1] += OPEN_ROOK_MOD * rooksOnFileB[i];
            }
        }
        if (rooksOnFileW[i]) {
            if (pawnsOnFileW[i] == 0) {
                val[0] += SEMI_OPEN_ROOK_MOD * rooksOnFileW[i];
                if (pawnsOnFileW[i] == 0)
                    val[0] += OPEN_ROOK_MOD * rooksOnFileW[i];
            }
        }
    }
    if (state->turn == WHITE) {
        return val[0] - val[1];
    }
    return val[1] - val[0];
}

#define MAX_DEPTH 20

static void
quicksort (struct move *A, int len)
{
    if (len < 2) return;

    int8_t pivot = A[len / 2].priority;

    int i, j;
    for (i = 0, j = len - 1;; i++, j--) {
        while (A[i].priority > pivot)
            i++;
        while (A[j].priority < pivot)
            j--;

        if (i >= j) break;

        struct move temp = A[i];
        A[i] = A[j];
        A[j] = temp;
    }

    quicksort (A, i);
    quicksort (A + i, len - i);
}

static int
alphabetaSearch (int alpha, int beta, uint8_t *board, struct gamestate *state,
                 int depth)
{
    if (depth <= 0) {
        return eval (board, state);
    }
    struct move moves[256] = { 0 };

    int n = searchMoves (board, *state, moves);
    if (n == 0) {
        if (testCheck (board,
                       (state->turn == WHITE) ? state->kingW : state->kingB,
                       state->turn)) {
            return -MATE;
        }
        return 0;
    }
    for (int i = 0; i < n; i++) {
        uint8_t nextBoard[BOARD_MEM_SIZE];
        memcpy (nextBoard, board, BOARD_MEM_SIZE);
        struct gamestate newState;
        newState = *state;
        makeMove (nextBoard, &newState, state->turn, moves[i]);
        newState.turn ^= COLOR_MASK;
        int val = -alphabetaSearch (-beta, -alpha, nextBoard, &newState,
                                    depth - 1);
        if (val > alpha) alpha = val;
        if (alpha >= beta) return beta;
    }
    return alpha;
}

#define MAX_TIME 500000

struct move
getBestMove (uint8_t *board, struct gamestate *state, uint maxdepth)
{
    struct move moves[250] = { 0 };
    uint n = searchMoves (board, *state, moves);
    uint bestmove = 0;
    uint startTime = clock ();
    uint depth = 2;
    while (depth <= maxdepth && clock () - startTime < MAX_TIME / 2) {
        int alpha = -MATE;
        int beta = MATE;
        for (uint i = 0; i < n; i++) {
            uint8_t nextBoard[BOARD_MEM_SIZE];
            memcpy (nextBoard, board, BOARD_MEM_SIZE);
            struct gamestate newState;
            newState = *state;
            newState.turn ^= COLOR_MASK;
            makeMove (nextBoard, &newState, state->turn, moves[i]);
            int val = -alphabetaSearch (-beta, -alpha, nextBoard, &newState,
                                        depth - 1);
            if (val > alpha) {
                alpha = val;
                bestmove = i;
            }
            if (alpha >= beta) return moves[bestmove];
        }
        depth++;
    }
    return moves[bestmove];
}
