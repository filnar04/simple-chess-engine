#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int8_t knightMoves[]
    = { -WIDTH + 2, -WIDTH - 2, -2 * WIDTH + 1, -2 * WIDTH - 1,
        WIDTH - 2,  WIDTH + 2,  2 * WIDTH + 1,  2 * WIDTH - 1 };
const int8_t bishopMoves[] = { -WIDTH + 1, WIDTH - 1, -WIDTH - 1, WIDTH + 1 };
const int8_t rookMoves[] = { -1, 1, -WIDTH, WIDTH };
const int8_t kingMoves[]
    = { -1, 1, -WIDTH + 1, WIDTH - 1, -WIDTH, WIDTH, -WIDTH - 1, WIDTH + 1 };

int8_t
testCheck (uint8_t *board, int8_t pos, int8_t color)
{
    int8_t attack_num = 0, attack_pos = VERTICAL_STEP;
    if (color == BLACK) attack_pos = -VERTICAL_STEP;
    attack_pos += pos + HORIZONTAL_STEP;
    // test if attacked by pawn
    if (!(board[attack_pos] & color)
        && (board[attack_pos] & PIECE_TYPE) == PAWN) {
        attack_num++;
    }
    if (!(board[attack_pos - 2] & color)
        && (board[attack_pos - 2] & PIECE_TYPE) == PAWN) {
        attack_num++;
    }

    for (int i = 0; i < KNIGHT_MOVES; i++) {
        attack_pos = pos + knightMoves[i];
        if ((board[attack_pos] & PIECE_TYPE) == KNIGHT
            && !(board[attack_pos] & color)) {
            attack_num++;
            break;
        }
    }
    for (int i = 0; i < 4; i++) {
        attack_pos = pos + bishopMoves[i];
        while (!board[attack_pos]) {
            attack_pos += bishopMoves[i];
        }
        int8_t piece = board[attack_pos] & PIECE_TYPE;
        if (!(board[attack_pos] & color)
            && (piece == BISHOP || piece == QUEEN)) {
            attack_num++;
            break;
        }
    }
    for (int i = 0; i < 4; i++) {
        attack_pos = pos + rookMoves[i];
        while (!board[attack_pos]) {
            attack_pos += rookMoves[i];
        }
        int8_t piece = board[attack_pos] & PIECE_TYPE;
        if (!(board[attack_pos] & color)
            && (piece == ROOK || piece == QUEEN)) {
            attack_num++;
            break;
        }
    }
    for (int i = 0; i < 8; i++) {
        attack_pos = pos + kingMoves[i];
        int8_t piece = board[attack_pos] & PIECE_TYPE;
        if (!(board[attack_pos] & color) && piece == KING) {
            attack_num++;
            break;
        }
    }

    return attack_num;
}

#define testPin(board, pos, kingPos, color, move, type)                       \
    int8_t step = move;                                                       \
    if (kingPos > pos) step = -step;                                          \
    next = pos + step;                                                        \
    while (!board[next]) {                                                    \
        next += step;                                                         \
    }                                                                         \
    if (!(board[next] & color)                                                \
        && ((board[next] & PIECE_TYPE) == type                                \
            || (board[next] & PIECE_TYPE) == QUEEN))                          \
        return 1;

int8_t
isPinned (uint8_t *board, int8_t pos, int8_t kingPos)
{
    int8_t next, color = board[pos] & COLOR_MASK;
    if (ranks[pos] == ranks[kingPos]) {
        testPin (board, pos, kingPos, color, 1, ROOK)
        // return 1;
    }
    if (files[pos] == files[kingPos]) {
        testPin (board, pos, kingPos, color, VERTICAL_STEP, ROOK)
        // return 1;
    }
    if (diagonals1[pos] == diagonals1[kingPos]) {
        testPin (board, pos, kingPos, color, VERTICAL_STEP + 1, BISHOP)
        // return 1;
    }
    if (diagonals2[pos] == diagonals2[kingPos]) {
        testPin (board, pos, kingPos, color, VERTICAL_STEP - 1, BISHOP)
        // return 1;
    }
    return 0;
}

int8_t
findPawnMoves (uint8_t *board, struct gamestate gameState, int8_t pos,
               int8_t *list)
{
    int8_t moveCount = 0, step = WIDTH;
    if (gameState.turn == BLACK) step = -WIDTH;
    if (!board[pos + step]) {
        list[moveCount++] = pos + step;
        if (!(board[pos] & HAS_MOVED) && !board[pos + (step << 1)])
            list[moveCount++] = pos + (step << 1);
    }
    int8_t next = pos + step + 1;
    for (int i = 0; i < 2; i++) {
        if (board[next] && !(board[next] & gameState.turn)) {
            list[moveCount++] = next;
        } else if (gameState.enpasssant == next) {
            int8_t tmp = board[next - step];
            board[next] = board[pos];
            board[pos] = 0;
            board[next - step] = 0;
            if (!testCheck (board,
                            (gameState.turn == WHITE) ? gameState.kingW
                                                      : gameState.kingB,
                            gameState.turn)) {
                list[moveCount++] = next;
            }
            board[pos] = board[next];
            board[next - step] = tmp;
            board[next] = 0;
        }
        next -= 2;
    }
    return moveCount;
}

int8_t
findKnightMoves (uint8_t *board, int8_t pos, int8_t color, int8_t *list)
{
    int8_t moveCount = 0;
    for (int i = 0; i < KNIGHT_MOVES; i++) {
        if (!(board[pos + knightMoves[i]] & color))
            list[moveCount++] = pos + knightMoves[i];
    }
    return moveCount;
}
#define findStraightMoves(type, count, pos, color, list)                      \
    int8_t move_count = 0;                                                    \
    for (int i = 0; i < count; i++) {                                         \
        int8_t next = pos + type[i];                                          \
        while (!board[next]) {                                                \
            list[move_count++] = next;                                        \
            next += type[i];                                                  \
        }                                                                     \
        if (!(board[next] & color)) list[move_count++] = next;                \
    }                                                                         \
    return move_count

int8_t
findBishopMoves (uint8_t *board, int8_t pos, int8_t color, int8_t *list)
{
    findStraightMoves (bishopMoves, 4, pos, color, list);
}

int8_t
findRookMoves (uint8_t *board, int8_t pos, int8_t color, int8_t *list)
{
    findStraightMoves (rookMoves, 4, pos, color, list);
}

int8_t
findQueenMoves (uint8_t *board, int8_t pos, int8_t color, int8_t *list)
{
    findStraightMoves (kingMoves, 8, pos, color, list);
}

int8_t
testCastling (uint8_t *board, int8_t king, int8_t rook)
{
    if ((board[rook] & (PIECE_TYPE | HAS_MOVED)) != ROOK) return 0;
    int8_t step = (king < rook) ? 1 : -1;
    for (int8_t i = king + step; i != rook; i += step) {
        if (board[i]) return 0;
    }
    for (int8_t j = 0; j < 3; j++) {
        if (testCheck (board, king + j * step, board[king] & COLOR_MASK))
            return 0;
    }
    return 1;
}

int8_t
findKingMoves (uint8_t *board, struct gamestate gameState, int8_t pos,
               int8_t *list)
{
    int8_t temp, next, moveCount = 0;
    for (int i = 0; i < 8; i++) {
        next = pos + kingMoves[i];
        if (!(board[next] & gameState.turn)) {
            temp = board[next];
            board[next] = board[pos];
            board[pos] = 0;
            if (!testCheck (board, next, gameState.turn)) {
                list[moveCount++] = next;
            }
            board[pos] = board[next];
            board[next] = temp;
        }
    }
    // TODO: do something with this mess
    if (gameState.shortcastle & gameState.turn) {
        /*if (!(board[pos + 1]) && !board[pos + 2]
            && (board[pos + 3] & (PIECE_TYPE | HAS_MOVED)) == ROOK
            && (board[pos + 3] & color))*/
        if (testCastling (board, pos, pos + 3)) {
            board[pos + 2] = board[pos];
            board[pos] = 0;
            board[pos + 1] = board[pos + 3];
            board[pos + 3] = 0;
            if (!(testCheck (board, pos + 2, gameState.turn)
                  || testCheck (board, pos + 1, gameState.turn)))
                list[moveCount++] = pos + 2;
            board[pos] = board[pos + 2];
            board[pos + 2] = 0;
            board[pos + 3] = board[pos + 1];
            board[pos + 1] = 0;
        }
    }
    if (gameState.longcastle & gameState.turn) {
        /*if (!(board[pos - 1]) && !(board[pos - 2]) && !(board[pos - 3])
            && (board[pos - 4] & (PIECE_TYPE | HAS_MOVED)) == ROOK
            && (board[pos - 4] & color))*/
        if (testCastling (board, pos, pos - 4)) {
            board[pos - 2] = board[pos];
            board[pos] = 0;
            board[pos - 1] = board[pos - 4];
            board[pos - 4] = 0;
            if (!(testCheck (board, pos - 2, gameState.turn)
                  || testCheck (board, pos - 1, gameState.turn)))
                list[moveCount++] = pos - 2;
            board[pos] = board[pos - 2];
            board[pos - 2] = 0;
            board[pos - 4] = board[pos - 1];
            board[pos - 1] = 0;
        }
    }
    return moveCount;
}

void
makeMove (uint8_t *board, struct gamestate *gameState, int8_t color,
          struct move m)
{
    gameState->halfmove += 1;
    gameState->enpasssant = 0;
    if ((board[m.start] & PIECE_TYPE) == PAWN) {
        gameState->halfmove = 0;
        if (m.promotion > 0) {
            board[m.end] = m.promotion | color; //| HAS_MOVED;
            board[m.start] = 0;
            return;
        } else if (files[m.start] != files[m.end]
                   && !board[m.end]) { // en passant
            int8_t enemy = (color == WHITE) ? m.end - VERTICAL_STEP
                                            : m.end + VERTICAL_STEP;
            board[enemy] = 0;
        }

        if (abs (m.end - m.start) == 2 * WIDTH) {
            gameState->enpasssant = ((short)m.end + m.start) / 2;
        }

    } else if ((board[m.start] & PIECE_TYPE) == KING) {
        if (color == WHITE)
            gameState->kingW = m.end;
        else
            gameState->kingB = m.end;
        gameState->shortcastle &= ~color;
        gameState->longcastle &= ~color;
        if (m.end - m.start == 2) { // short castle
            board[m.start + 1] = board[m.end + 1] | HAS_MOVED;
            board[m.end + 1] = 0;
        } else if (m.end - m.start == -2) {
            board[m.start - 1] = board[m.end - 2] | HAS_MOVED;
            board[m.end - 2] = 0;
        }
    }
    if (board[m.end]) gameState->halfmove = 0;
    board[m.end] = board[m.start] | HAS_MOVED;
    board[m.start] = 0;
}

unsigned int
findPieceMoves (uint8_t *board, struct gamestate gameState, int8_t square,
                int8_t inCheck, struct move *moveList)
{
    unsigned int numMoves;
    int8_t tempMoveList[30];
    int8_t color = gameState.turn;
    int8_t type = board[square] & PIECE_TYPE;
    switch (type) {
    case PAWN:
        numMoves = findPawnMoves (board, gameState, square, tempMoveList);
        break;
    case KNIGHT:
        numMoves = findKnightMoves (board, square, color, tempMoveList);
        break;
    case BISHOP:
        numMoves = findBishopMoves (board, square, color, tempMoveList);
        break;
    case ROOK:
        numMoves = findRookMoves (board, square, color, tempMoveList);
        break;
    case QUEEN:
        numMoves = findQueenMoves (board, square, color, tempMoveList);
        break;
    case KING:
        numMoves = findKingMoves (board, gameState, square, tempMoveList);
        break;
    // Should never happen
    default:
        printf ("Wrong piece type: %d at (%d)", board[square] & PIECE_TYPE,
                square);
        exit (EXIT_FAILURE);
    }
    if (numMoves == 0) return 0;

    int8_t kingPos = (color == WHITE) ? gameState.kingW : gameState.kingB;
    int8_t pinned = isPinned (board, square, kingPos);
    int8_t numLegal = 0;
    if (inCheck || pinned) {
        for (int i = 0; i < numMoves; i++) {
            if (type == PAWN && tempMoveList[i] == gameState.enpasssant) {
                moveList[numLegal].end = tempMoveList[i];
                moveList[numLegal].start = square;
                numLegal++;
                continue;
            }
            int8_t tmp = board[tempMoveList[i]];
            board[tempMoveList[i]] = board[square];
            board[square] = 0;
            int8_t tmpKingPos = (type == KING) ? tempMoveList[i] : kingPos;
            if (testCheck (board, tmpKingPos, color) == 0) {
                moveList[numLegal].end = tempMoveList[i];
                moveList[numLegal].start = square;
                numLegal++;
            }
            board[square] = board[tempMoveList[i]];
            board[tempMoveList[i]] = tmp;
        }
    } else {
        numLegal = numMoves;
        for (int i = 0; i < numLegal; i++) {
            moveList[i].start = square;
            moveList[i].end = tempMoveList[i];
        }
    }
    return numLegal;
}

unsigned int
searchMoves (uint8_t *board, struct gamestate gameState, struct move *moveList)
{
    int8_t kingPos
        = (gameState.turn == WHITE) ? gameState.kingW : gameState.kingB;
    int8_t attackerNum = testCheck (board, kingPos, gameState.turn);
    unsigned int moveCount = 0;

    if (attackerNum > 1) {
        // king attacked by 2 enemy pieces -> the king MUST move
        int8_t kingMoveList[8];
        int nmoves = findKingMoves (board, gameState, kingPos, kingMoveList);
        for (int i = 0; i < nmoves; i++) {
            moveList->end = kingMoveList[i];
            moveList->start = kingPos;
        }
    }
    int8_t promotionRank
        = (gameState.turn == WHITE) ? WHITE_PROMOTION : BLACK_PROMOTION;
    for (int r = 1; r <= 8; r++) {
        for (int c = 1; c <= 8; c++) {
            int8_t square = WIDTH * (r + 1) + c;
            int8_t piece = board[square];
            if (!(piece & gameState.turn)) continue;
            int8_t numMoves = 0;
            struct move *currMoveList = moveList + moveCount;

            numMoves = findPieceMoves (board, gameState, square, attackerNum,
                                       currMoveList);
            if (numMoves == 0) continue;
            if ((piece & PIECE_TYPE) == PAWN
                && ranks[currMoveList[0].end] == promotionRank) {
                for (int i = numMoves - 1; i >= 0; i--) {
                    currMoveList[i * 4].start = currMoveList[i].start;
                    currMoveList[i * 4].end = currMoveList[i].end;
                    currMoveList[i * 4].promotion = QUEEN;
                    for (int j = 1; j <= 3; j++) {
                        currMoveList[i * 4 + j].promotion = PAWN + j;
                        currMoveList[i * 4 + j].end = currMoveList[i].end;
                        currMoveList[i * 4 + j].start = currMoveList[i].start;
                    }
                }
                moveCount += 4 * numMoves; // 4 possible promotions
            } else {
                moveCount += numMoves;
            }
        }
    }
    return moveCount;
}

int
getMoveArray (uint8_t *board, struct gamestate gameState, int8_t square,
              int8_t inCheck, uint8_t moveArr[64])
{
    struct move moves[300];
    int moveNum = findPieceMoves (board, gameState, square, inCheck, moves);
    for (int i = 0; i < moveNum; i++) {
        moveArr[LOGIC_TO_BOARD (moves[i].end)] = 1;
    }
    return moveNum;
}
