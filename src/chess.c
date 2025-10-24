// #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"


int8 currentBoard[120] = STARTING_POS;
// int8 testBoard[120];
int8 ranks[120], files[120], diagonals1[120], diagonals2[120];

int8 testCheck(int8* board, int8 pos, int8 color /*, int8* attackList*/) {
    int8 attackerCount = 0, attackPos = WIDTH;
    if (color == WHITE) attackPos = -WIDTH;
    attackPos += pos + 1;
    //test if attacked by pawn
    if (!(board[attackPos] & color) && (board[attackPos]|PIECE_TYPE) == pawn) {
        attackerCount++;
    } else if (!(board[attackPos - 2] & color) && (board[attackPos - 2]|PIECE_TYPE) == pawn) {
        attackerCount++;
    } else { //it's impossible to be checked by a pawn and another piece at the same time
        for (int i = 0; i < KNIGHT_MOVES; i++) {
            attackPos = pos + knightMoves[i];
            if ((board[attackPos]|PIECE_TYPE) == knight && !(board[attackPos] & color)) {
                attackerCount++;
                break;
            }
        }
        for (int i = 0; i < 4; i++) {
            do {
                attackPos = pos + bishopMoves[i];
            } while (!board[attackPos]);
            int8 piece = board[attackPos]|PIECE_TYPE;
            if ((board[attackPos] & color) && (piece == bishop || piece == queen)) {
                attackerCount++;
                break;
            }
        }
        for (int i = 0; i < 4; i++) {
            do {
                attackPos = pos + rookMoves[i];
            } while (!board[attackPos]);
            int8 piece = board[attackPos]|PIECE_TYPE;
            if ((board[attackPos] & color) && (piece == rook || piece == queen)) {
                attackerCount++;
                break;
            }
        }
    }

    return attackerCount;
}

#define testPin(board, pos, kingPos, color, move, type) \
    int8 step = move; \
    if (kingPos > pos) step = -step; \
    next = pos + step; \
    while (!board[step]) { \
    next += step; \
    } \
    if (!(board[next] & color) && (board[next] == type || board[next] == queen)) return 1; 

int8 isPinned(int8* board, int8 pos, int8 kingPos) {
    int8 next, color = board[pos] & COLOR_MASK;
    if (ranks[pos] == ranks[kingPos]) {
        testPin(board, pos, kingPos, color, 1, rook) return 1;
    }
    if (files[pos] == files[kingPos]) {
        testPin(board, pos, kingPos, color, VERTICAL_STEP, rook) return 2;
    }
    if (diagonals1[pos] == diagonals1[kingPos]) {
        testPin(board, pos, kingPos, color, VERTICAL_STEP + 1, bishop) return 3;
    }
    if (diagonals2[pos] == diagonals2[kingPos]) {
        testPin(board, pos, kingPos, color, VERTICAL_STEP - 1, bishop) return 3;
    }
    return 0;
}

int8 findPawnMoves(int8 *board, int8 *gameState, int8 pos, int8 color, int8 *list) {
    int8 moveCount = 0, step = WIDTH;
    if (color == BLACK) step = -WIDTH;
    if (!board[pos + step]) {
        list[moveCount++] = pos + step;
        if (!(board[pos] & HAS_MOVED) && !board[pos + (step << 1)])
            list[moveCount++] = pos + (step << 1);
    }
    int8 next = pos + step + 1;
    if (board[next] && !(board[next] & color)) list[moveCount++] = next;
    next -= 2;
    if (board[next] && !(board[next] & color)) list[moveCount++] = next;
    return moveCount;
}

int8 findKnightMoves(int8* board, int8 *gameState, int8 pos, int8 color, int8 *list) {
    int8 moveCount = 0;
    for (int i = 0; i < KNIGHT_MOVES; i++) {
        if (!(board[pos + knightMoves[i]] & color)) list[moveCount++] = pos + knightMoves[i];
    }
    return moveCount;
}
#define findStraightMoves(type, count, pos, color, list) \
    int8 moveCount = 0; \
    for (int i = 0; i < count; i++) { \
        int8 next = pos + type[i];   \
        while (!board[next]) {  \
            list[moveCount++] = next; \
            next += type[i]; \
        } \
        if (!(board[next] & color)) list[moveCount++] = next; \
    } \
    return moveCount;

int8 findBishopMoves(int8 *board, int8 *gameState,int8 pos, int8 color, int8 *list) {
    findStraightMoves(bishopMoves, 4, pos, color, list)
}

int8 findRookMoves(int8 *board, int8 *gameState,int8 pos, int8 color, int8 *list) {
    findStraightMoves(rookMoves, 4, pos, color, list)
}

int8 findQueenMoves(int8 *board, int8 *gameState,int8 pos, int8 color, int8 *list) {
    findStraightMoves(kingMoves, 8, pos, color, list)
}
int8 findKingMoves(int8 *board, int8 *gameState, int8 pos, int8 color, int8 *list) {
    // memcpy(testBoard, board, BOARD_MEM_SIZE);
    int8 temp, next, moveCount = 0, isblack = (color == BLACK);
    for (int i = 0; i < 8; i++) {
        next = pos + kingMoves[i];
        if (!(board[next] & color)) {
            temp = board[next];
            board[next] = board[pos];
            board[pos] = 0;
            if (!testCheck(board, next, color)) {
                list[moveCount++] = next;
            }
            board[next] = temp;
        }
    }
    if (gameState[SHORTCASTLE_W + isblack]) {
        if (!(board[pos + 1]) && !board[pos + 2]) {
            board[pos + 2] = board[pos];
            board[pos] = 0;
            board[pos + 1] = board[pos + 3];
            board[pos + 3] = 0;
            if (!(testCheck(board, pos + 2, color) || testCheck(board, pos + 1, color))) list[moveCount++] = pos + 2;
            for (int i = 0; i < 4; i++) board[pos + i] = board[pos + i];
        }
    }
    if (gameState[LONGCASTLE_W + isblack]) {
        if (!(board[pos - 1]) && !(board[pos - 2] && !(board[pos - 3]))) {
            board[pos - 2] = board[pos];
            board[pos] = 0;
            board[pos - 1] = board[pos - 4];
            board[pos - 4] = 0;
            if (!(testCheck(board, pos - 2, color) || testCheck(board, pos - 1, color))) list[moveCount++] = pos - 2;
            for (int i = 0; i >= -4; i--) board[pos + i] = board[pos + i];
 
        }
    }
    
    return moveCount;
}

int8 (*find[6])(int8*, int8*, int8, int8, int8*) = {findPawnMoves, findKnightMoves, findBishopMoves, findRookMoves, findQueenMoves};

void makeMove(int8 *board, int8 *gameState, int8 start, int8 end) {
    gameState[HALFMOVE_CLOCK] += 1;
    if ((board[start]&PIECE_TYPE) == pawn) {
        gameState[HALFMOVE_CLOCK] = 0;
        if (abs(end - start) > WIDTH) {
            gameState[ENPASSANT_POS] = ((short)end + start) / 2; 
        }
    } else if ((board[start]&PIECE_TYPE) == king) {
        int8 isblack = (board[start]&BLACK) > 0;
        gameState[SHORTCASTLE_W + isblack] = 0;
        gameState[LONGCASTLE_W + isblack] = 0;
        if (end - start == 2) { //short castle
            board[start + 1] = board[end + 1]|HAS_MOVED;
            board[end + 1] = 0;
        } else if (end - start == -2) {
            board[start - 1] = board[end - 2]|HAS_MOVED;
            board[end - 2] = 0;
        }
    } 
    if (board[end]) gameState[HALFMOVE_CLOCK] = 0;
    board[end] = board[start]|HAS_MOVED;
    board[start] = 0;
    
}

unsigned int searchMoves(int8* board, int8* gameState, int8 color, int8* moveList) {
    int8 attackerNum = testCheck(board, gameState[KING_POS_W + (color == BLACK)], color),
        kingPos = gameState[KING_POS_W + (color == BLACK)],
        tempMoveList[30];
    unsigned int moveCount = 0;
    if (attackerNum > 1) {//king attacked by 2 enemy pieces -> the king MUST move
        return findKingMoves(board, gameState, kingPos, color, moveList);
    }
    for (int r = 1; r <= 8; r++) {
        for (int c = 1; c <= 8; c++) {
            int8 pos = WIDTH * r + c;
            if (board[pos]&color) {
                int8 pinned = isPinned(board, pos, kingPos);
                int8 numMoves = find[board[pos]&PIECE_TYPE](board, gameState, pos, color, tempMoveList);
                if (attackerNum || pinned) {
                    for (int i = 0; i < numMoves; i++) {
                        int8 tmp = board[tempMoveList[i]];
                        board[tempMoveList[i]] = board[pos];
                        board[pos] = 0;
                        if (!testCheck(board, kingPos, color)) {
                            moveList[moveCount++] = tempMoveList[i];
                        }
                        board[pos] = board[tempMoveList[i]];
                        board[tempMoveList[i]] = tmp;
                    }
                } else {
                    memcpy(moveList + moveCount, tempMoveList, numMoves);
                    moveCount += numMoves;
                }
            }
        } 
    }
    return moveCount;
}
