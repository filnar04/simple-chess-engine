#include "util.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int8 currentBoard[120] = STARTING_POS;

int8
testCheck (int8 *board, int8 pos, int8 color)
{
    int8 attack_num = 0, attack_pos = VERTICAL_STEP;
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
        int8 piece = board[attack_pos] & PIECE_TYPE;
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
        int8 piece = board[attack_pos] & PIECE_TYPE;
        if (!(board[attack_pos] & color)
            && (piece == ROOK || piece == QUEEN)) {
            attack_num++;
            break;
        }
    }
    for (int i = 0; i < 8; i++) {
        attack_pos = pos + kingMoves[i];
        int8 piece = board[attack_pos] & PIECE_TYPE;
        if (!(board[attack_pos] & color) && piece == KING) {
            attack_num++;
            break;
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
        && ((board[next] & PIECE_TYPE) == type                                \
            || (board[next] & PIECE_TYPE) == QUEEN))                          \
        return 1;

int8
isPinned (int8 *board, int8 pos, int8 kingPos)
{
    int8 next, color = board[pos] & COLOR_MASK;
    if (ranks[pos] == ranks[kingPos]) {
        // testPin (board, pos, kingPos, color, 1, ROOK)
        return 1;
    }
    if (files[pos] == files[kingPos]) {
        // testPin (board, pos, kingPos, color, VERTICAL_STEP, ROOK)
        return 1;
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

int8
findPawnMoves (int8 *board, struct gamestate gameState, int8 pos, int8 color,
               int8 *list)
{
    int8 moveCount = 0, step = WIDTH;
    if (color == BLACK) step = -WIDTH;
    if (!board[pos + step]) {
        list[moveCount++] = pos + step;
        if (!(board[pos] & HAS_MOVED) && !board[pos + (step << 1)])
            list[moveCount++] = pos + (step << 1);
    }
    int8 next = pos + step + 1; /*
     if ((board[next] && !(board[next] & color))
         || gameState.enpasssant == next)
         list[moveCount++] = next;
     next -= 2;*/
    for (int i = 0; i < 2; i++) {
        if (board[next] && !(board[next] & color)) {
            list[moveCount++] = next;
        } else if (gameState.enpasssant == next) {
            int8 tmp = board[next - step];
            board[next] = board[pos];
            board[pos] = 0;
            board[next - step] = 0;
            if (!testCheck (board,
                            (color == WHITE) ? gameState.kingW
                                             : gameState.kingB,
                            color)) {
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
testCastling (int8 *board, int8 king, int8 rook)
{
    if ((board[rook] & (PIECE_TYPE | HAS_MOVED)) != ROOK) return 0;
    int8 step = (king < rook) ? 1 : -1;
    for (int8 i = king + step; i != rook; i += step) {
        if (board[i]) return 0;
    }
    for (int8 j = 0; j < 3; j++) {
        if (testCheck (board, king + j * step, board[king] & COLOR_MASK))
            return 0;
    }
    return 1;
}

int8
findKingMoves (int8 *board, struct gamestate gameState, int8 pos, int8 color,
               int8 *list)
{
    int8 temp, next, moveCount = 0;
    for (int i = 0; i < 8; i++) {
        next = pos + kingMoves[i];
        if (!(board[next] & color)) {
            temp = board[next];
            board[next] = board[pos];
            board[pos] = 0;
            if (!testCheck (board, next, color)) {
                list[moveCount++] = next;
            }
            board[pos] = board[next];
            board[next] = temp;
        }
    }
    // TODO: do something with this mess
    if (gameState.shortcastle & color) {
        /*if (!(board[pos + 1]) && !board[pos + 2]
            && (board[pos + 3] & (PIECE_TYPE | HAS_MOVED)) == ROOK
            && (board[pos + 3] & color))*/
        if (testCastling (board, pos, pos + 3)) {
            board[pos + 2] = board[pos];
            board[pos] = 0;
            board[pos + 1] = board[pos + 3];
            board[pos + 3] = 0;
            if (!(testCheck (board, pos + 2, color)
                  || testCheck (board, pos + 1, color)))
                list[moveCount++] = pos + 2;
            board[pos] = board[pos + 2];
            board[pos + 2] = 0;
            board[pos + 3] = board[pos + 1];
            board[pos + 1] = 0;
        }
    }
    if (gameState.longcastle & color) {
        /*if (!(board[pos - 1]) && !(board[pos - 2]) && !(board[pos - 3])
            && (board[pos - 4] & (PIECE_TYPE | HAS_MOVED)) == ROOK
            && (board[pos - 4] & color))*/
        if (testCastling (board, pos, pos - 4)) {
            board[pos - 2] = board[pos];
            board[pos] = 0;
            board[pos - 1] = board[pos - 4];
            board[pos - 4] = 0;
            if (!(testCheck (board, pos - 2, color)
                  || testCheck (board, pos - 1, color)))
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
makeMove (int8 *board, struct gamestate *gameState, int8 start, int8 end,
          int8 promotion)
{
    int8 color = board[start] & COLOR_MASK;
    gameState->halfmove += 1;
    gameState->enpasssant = 0;
    if ((board[start] & PIECE_TYPE) == PAWN) {
        gameState->halfmove = 0;
        if (promotion > 0) {
            board[end] = promotion | color; //| HAS_MOVED;
            board[start] = 0;
            return;
        } else if (files[start] != files[end] && !board[end]) { // en passant
            int8 enemy
                = (color == WHITE) ? end - VERTICAL_STEP : end + VERTICAL_STEP;
            board[enemy] = 0;
        }

        if (abs (end - start) == 2 * WIDTH) {
            gameState->enpasssant = ((short)end + start) / 2;
        }

    } else if ((board[start] & PIECE_TYPE) == KING) {
        if (color == WHITE)
            gameState->kingW = end;
        else
            gameState->kingB = end;
        gameState->shortcastle &= ~color;
        gameState->longcastle &= ~color;
        if (end - start == 2) { // short castle
            board[start + 1] = board[end + 1] | HAS_MOVED;
            board[end + 1] = 0;
        } else if (end - start == -2) {
            board[start - 1] = board[end - 2] | HAS_MOVED;
            board[end - 2] = 0;
        }
    }
    if (board[end]) gameState->halfmove = 0;
    board[end] = board[start] | HAS_MOVED;
    board[start] = 0;
}

unsigned int
findPieceMoves (int8 *board, struct gamestate gameState, int8 square,
                int8 inCheck, int8 *moveList)
{
    unsigned int numMoves;
    int8 tempMoveList[30];
    int8 color = board[square] & COLOR_MASK;
    int8 type = board[square] & PIECE_TYPE;
    switch (type) {
    case PAWN:
        numMoves
            = findPawnMoves (board, gameState, square, color, tempMoveList);
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
        numMoves
            = findKingMoves (board, gameState, square, color, tempMoveList);
        break;
    // Should never happen
    default:
        printf ("Wrong piece type: %d at (%d)", board[square] & PIECE_TYPE,
                square);
        exit (EXIT_FAILURE);
    }
    if (numMoves == 0) return 0;

    int8 kingPos = (color == WHITE) ? gameState.kingW : gameState.kingB;
    int8 pinned = isPinned (board, square, kingPos);
    int8 numLegal = 0;
    if (inCheck || pinned) {
        for (int i = 0; i < numMoves; i++) {
            int8 tmp = board[tempMoveList[i]];
            board[tempMoveList[i]] = board[square];
            board[square] = 0;
            int8 tmpKingPos = (type == KING) ? tempMoveList[i] : kingPos;
            if (testCheck (board, tmpKingPos, color) == 0) {
                moveList[numLegal++] = tempMoveList[i];
            }
            board[square] = board[tempMoveList[i]];
            board[tempMoveList[i]] = tmp;
        }
    } else {
        numLegal = numMoves;
        memcpy (moveList, tempMoveList, numLegal);
    }
    return numLegal;
}

unsigned int
searchMoves (int8 *board, struct gamestate gameState, int8 color,
             int8 *moveList, int8 *pieceList)
{
    int8 kingPos = (color == WHITE) ? gameState.kingW : gameState.kingB;
    int8 attackerNum = testCheck (board, kingPos, color);
    unsigned int moveCount = 0;

    if (attackerNum > 1) {
        // king attacked by 2 enemy pieces -> the king MUST move
        pieceList[0] = kingPos;
        return findKingMoves (board, gameState, kingPos, color, moveList);
    }
    int8 promotionRank = (color == WHITE) ? WHITE_PROMOTION : BLACK_PROMOTION;
    for (int r = 1; r <= 8; r++) {
        for (int c = 1; c <= 8; c++) {
            int8 square = WIDTH * (r + 1) + c;
            int8 piece = board[square];
            if (!(piece & color)) continue;
            int8 numMoves = 0;
            int8 *currMoveList = moveList + moveCount;

            numMoves = findPieceMoves (board, gameState, square, attackerNum,
                                       currMoveList);
            if (numMoves == 0) continue;
            pieceList[moveCount] = square;
            if ((piece & PIECE_TYPE) == PAWN
                && ranks[currMoveList[0]] == promotionRank) {
                int8 temp[3];
                memcpy (temp, moveList, numMoves);
                for (int i = 0; i < numMoves; i++) {
                    moveList[moveCount++] = temp[i];
                    for (int j = 0; j < 3; j++) { // underpromotion
                        pieceList[moveCount] = KNIGHT + j;
                        moveList[moveCount++] = temp[i];
                    }
                }
            } else {
                moveCount += numMoves;
            }
        }
    }
    return moveCount;
}
