#include "util.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int8 currentBoard[120] = STARTING_POS;

int8
testCheck (int8 *board, int8 pos, int8 color)
{
    int8 attack_num = 0, attack_pos = WIDTH;
    if (color == WHITE) attack_pos = -WIDTH;
    attack_pos += pos + 1;
    // test if attacked by pawn
    if (!(board[attack_pos] & color)
        && (board[attack_pos] | PIECE_TYPE) == PAWN) {
        attack_num++;
    } else if (!(board[attack_pos - 2] & color)
               && (board[attack_pos - 2] & PIECE_TYPE) == PAWN) {
        attack_num++;
    } else { // it's impossible to be checked by a pawn and another piece at
             // the same time
        for (int i = 0; i < KNIGHT_MOVES; i++) {
            attack_pos = pos + knightMoves[i];
            if ((board[attack_pos] & PIECE_TYPE) == KNIGHT
                && !(board[attack_pos] & color)) {
                attack_num++;
                break;
            }
        }
        for (int i = 0; i < 4; i++) {
            do {
                attack_pos = pos + bishopMoves[i];
            } while (!board[attack_pos]);
            int8 piece = board[attack_pos] | PIECE_TYPE;
            if ((board[attack_pos] & color)
                && (piece == BISHOP || piece == QUEEN)) {
                attack_num++;
                break;
            }
        }
        for (int i = 0; i < 4; i++) {
            do {
                attack_pos = pos + rookMoves[i];
            } while (!board[attack_pos]);
            int8 piece = board[attack_pos] | PIECE_TYPE;
            if ((board[attack_pos] & color)
                && (piece == ROOK || piece == QUEEN)) {
                attack_num++;
                break;
            }
        }
    }

    return attack_num;
}

#define testPin(board, pos, kingPos, color, move, type)                       \
    int8 step = move;                                                         \
    if (kingPos > pos) step = -step;                                          \
    next = pos + step;                                                        \
    while (!board[next]) {                                                    \
        next += step;                                                         \
    }                                                                         \
    if (!(board[next] & color)                                                \
        && (board[next] == type || board[next] == QUEEN))                     \
        return 1;

int8
isPinned (int8 *board, int8 pos, int8 kingPos)
{
    int8 next, color = board[pos] & COLOR_MASK;
    if (ranks[pos] == ranks[kingPos]) {
        testPin (board, pos, kingPos, color, 1, ROOK)
    }
    if (files[pos] == files[kingPos]) {
        testPin (board, pos, kingPos, color, VERTICAL_STEP, ROOK)
    }
    if (diagonals1[pos] == diagonals1[kingPos]) {
        testPin (board, pos, kingPos, color, VERTICAL_STEP + 1, BISHOP)
    }
    if (diagonals2[pos] == diagonals2[kingPos]) {
        testPin (board, pos, kingPos, color, VERTICAL_STEP - 1, BISHOP)
    }
    return 0;
}

int8
findPawnMoves (int8 *board, int8 *gameState, int8 pos, int8 color, int8 *list)
{
    int8 moveCount = 0, step = WIDTH;
    if (color == BLACK) step = -WIDTH;
    if (!board[pos + step]) {
        list[moveCount++] = pos + step;
        if (!(board[pos] & HAS_MOVED) && !board[pos + (step << 1)])
            list[moveCount++] = pos + (step << 1);
    }
    int8 next = pos + step + 1;
    if ((board[next] && !(board[next] & color))
        || gameState[ENPASSANT_POS] == next)
        list[moveCount++] = next;
    next -= 2;
    if ((board[next] && !(board[next] & color)
         || gameState[ENPASSANT_POS] == next))
        list[moveCount++] = next;
    return moveCount;
}

int8
findKnightMoves (int8 *board, int8 pos, int8 color, int8 *list)
{
    int8 moveCount = 0;
    for (int i = 0; i < KNIGHT_MOVES; i++) {
        if (!(board[pos + knightMoves[i]] & color))
            list[moveCount++] = pos + knightMoves[i];
    }
    return moveCount;
}
#define findStraightMoves(type, count, pos, color, list)                      \
    int8 move_count = 0;                                                      \
    for (int i = 0; i < count; i++) {                                         \
        int8 next = pos + type[i];                                            \
        while (!board[next]) {                                                \
            list[move_count++] = next;                                        \
            next += type[i];                                                  \
        }                                                                     \
        if (!(board[next] & color)) list[move_count++] = next;                \
    }                                                                         \
    return move_count

int8
findBishopMoves (int8 *board, int8 pos, int8 color, int8 *list)
{
    findStraightMoves (bishopMoves, 4, pos, color, list);
}

int8
findRookMoves (int8 *board, int8 pos, int8 color, int8 *list)
{
    findStraightMoves (rookMoves, 4, pos, color, list);
}

int8
findQueenMoves (int8 *board, int8 pos, int8 color, int8 *list)
{
    findStraightMoves (kingMoves, 8, pos, color, list);
}

int8
findKingMoves (int8 *board, int8 *gameState, int8 pos, int8 color, int8 *list)
{
    int8 temp, next, moveCount = 0, isblack = (color == BLACK);
    for (int i = 0; i < 8; i++) {
        next = pos + kingMoves[i];
        if (!(board[next] & color)) {
            temp = board[next];
            board[next] = board[pos];
            board[pos] = 0;
            if (!testCheck (board, next, color)) {
                list[moveCount++] = next;
            }
            board[next] = temp;
        }
    }
    // TODO: do something with this mess
    if (gameState[SHORTCASTLE_W + isblack]) {
        if (!(board[pos + 1]) && !board[pos + 2]) {
            board[pos + 2] = board[pos];
            board[pos] = 0;
            board[pos + 1] = board[pos + 3];
            board[pos + 3] = 0;
            if (!(testCheck (board, pos + 2, color)
                  || testCheck (board, pos + 1, color)))
                list[moveCount++] = pos + 2;
            for (int i = 0; i < 4; i++)
                board[pos + i] = board[pos + i];
        }
    }
    if (gameState[LONGCASTLE_W + isblack]) {
        if (!(board[pos - 1]) && !(board[pos - 2] && !(board[pos - 3]))) {
            board[pos - 2] = board[pos];
            board[pos] = 0;
            board[pos - 1] = board[pos - 4];
            board[pos - 4] = 0;
            if (!(testCheck (board, pos - 2, color)
                  || testCheck (board, pos - 1, color)))
                list[moveCount++] = pos - 2;
            for (int i = 0; i >= -4; i--)
                board[pos + i] = board[pos + i];
        }
    }

    return moveCount;
}

void
makeMove (int8 *board, int8 *gameState, int8 start, int8 end, int8 promotion)
{
    gameState[HALFMOVE_CLOCK] += 1;
    if ((board[start] & PIECE_TYPE) == PAWN) {
        gameState[HALFMOVE_CLOCK] = 0;
        if (promotion > 0) {
            board[end] = (board[start] & CLEAR_TYPE) | promotion | HAS_MOVED;
            board[start] = 0;
            return;
        }
        if (abs (end - start) == 2 * WIDTH) {
            gameState[ENPASSANT_POS] = ((short)end + start) / 2;
        }

    } else if ((board[start] & PIECE_TYPE) == KING) {
        int8 isblack = (board[start] & BLACK) > 0;
        gameState[SHORTCASTLE_W + isblack] = 0;
        gameState[LONGCASTLE_W + isblack] = 0;
        if (end - start == 2) { // short castle
            board[start + 1] = board[end + 1] | HAS_MOVED;
            board[end + 1] = 0;
        } else if (end - start == -2) {
            board[start - 1] = board[end - 2] | HAS_MOVED;
            board[end - 2] = 0;
        }
    }
    if (board[end]) gameState[HALFMOVE_CLOCK] = 0;
    board[end] = board[start] | HAS_MOVED;
    board[start] = 0;
}
unsigned int
searchMoves (int8 *board, int8 *gameState, int8 color, int8 *moveList,
             int8 *pieceList)
{
    int8 kingPos = gameState[KING_POS_W + (color == BLACK)];
    int8 attackerNum = testCheck (board, kingPos, color), tempMoveList[30];
    unsigned int moveCount = 0;

    if (attackerNum > 1) {
        // king attacked by 2 enemy pieces -> the king MUST move
        pieceList[0] = kingPos;
        return findKingMoves (board, gameState, kingPos, color, moveList);
    }
    for (int r = 1; r <= 8; r++) {
        for (int c = 1; c <= 8; c++) {
            int8 pos = WIDTH * (r + 1) + c;
            int8 piece = board[pos];
            if (!(piece & color)) continue;
            int8 pinned = isPinned (board, pos, kingPos);
            int8 numMoves = 0, promotionRank = 0;

            // pieceList[0] = pos;
            switch (piece & PIECE_TYPE) {
            case PAWN:
                numMoves = findPawnMoves (board, gameState, pos, color,
                                          tempMoveList);
                if (color == WHITE)
                    promotionRank = WHITE_PROMOTION;
                else
                    promotionRank = BLACK_PROMOTION;
                break;
            case KNIGHT:
                numMoves = findKnightMoves (board, pos, color, tempMoveList);
                break;
            case BISHOP:
                numMoves = findBishopMoves (board, pos, color, tempMoveList);
                break;
            case ROOK:
                numMoves = findRookMoves (board, pos, color, tempMoveList);
                break;
            case QUEEN:
                numMoves = findQueenMoves (board, pos, color, tempMoveList);
                break;
            case KING:
                numMoves = findKingMoves (board, gameState, pos, color,
                                          tempMoveList);
                break;
            // Should never happen
            default:
                printf ("Wrong piece type: %d at (%c %d)", piece & PIECE_TYPE,
                        'a' + c - 1, r);
                exit (EXIT_FAILURE);
            }
            if (numMoves == 0) continue;
            // TODO: split this mess into more functions
            if (attackerNum || pinned) {
                int8 legalNum = 0;
                for (int i = 0; i < numMoves; i++) {
                    int8 tmp = board[tempMoveList[i]];
                    board[tempMoveList[i]] = board[pos];
                    board[pos] = 0;

                    if (!testCheck (board, kingPos, color)) {

                        moveList[moveCount + legalNum] = tempMoveList[i];
                        legalNum++;
                        // pawn promotion
                        if ((piece & PIECE_TYPE) == PAWN
                            && ranks[tempMoveList[i]] == promotionRank) {
                            for (int j = 0; j < 3; j++) {
                                pieceList[moveCount + legalNum] = KNIGHT + j;
                                moveList[moveCount + legalNum]
                                    = tempMoveList[i];
                                legalNum++;
                            }
                        }
                    }
                    board[pos] = board[tempMoveList[i]];
                    board[tempMoveList[i]] = tmp;
                }
                moveCount += legalNum;
                if (legalNum > 0) pieceList[moveCount] = pos;
            } else {
                pieceList[moveCount] = pos;
                if ((piece & PIECE_TYPE) == PAWN
                    && ranks[tempMoveList[0]] == promotionRank) {
                    for (int i = 0; i < numMoves; i++) {
                        moveList[moveCount++] = tempMoveList[i];
                        for (int j = 0; j < 3; j++) {
                            pieceList[moveCount] = KNIGHT + j;
                            moveList[moveCount++] = tempMoveList[i];
                        }
                    }
                } else {
                    memcpy (moveList + moveCount, tempMoveList, numMoves);
                    moveCount += numMoves;
                }
            }
        }
    }
    return moveCount;
}
