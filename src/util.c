#pragma once
#include <stdint.h>

#define WIDTH 10
#define BOARD_SIZE 8
#define BOARD_MEM_SIZE 120
#define FIRST_SQUARE 21
#define LAST_SQUARE 99
#define WHITE_PROMOTION 8
#define BLACK_PROMOTION 1

#define HORIZONTAL_STEP 1
#define VERTICAL_STEP 10
#define KNIGHT_MOVES 8

enum pieces { PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING };
#define PIECE_TYPE 7
#define COLOR_MASK 060
#define WHITE 020
#define BLACK 040
#define HAS_MOVED 010
#define CLEAR_TYPE 0xf8

struct move {
    int8_t start;
    int8_t end;
    int8_t promotion;
    int8_t priority;
};

struct gamestate {
    int8_t turn;
    int8_t kingW;
    int8_t kingB;
    int8_t shortcastle;
    int8_t longcastle;
    int8_t enpasssant;
    int8_t halfmove;
};

#define WHITE_PAWN_START 2
#define BLACK_PAWN_START 7
const int8_t knightMoves[]
    = { -WIDTH + 2, -WIDTH - 2, -2 * WIDTH + 1, -2 * WIDTH - 1,
        WIDTH - 2,  WIDTH + 2,  2 * WIDTH + 1,  2 * WIDTH - 1 };
const int8_t bishopMoves[] = { -WIDTH + 1, WIDTH - 1, -WIDTH - 1, WIDTH + 1 };
const int8_t rookMoves[] = { -1, 1, -WIDTH, WIDTH };
const int8_t kingMoves[]
    = { -1, 1, -WIDTH + 1, WIDTH - 1, -WIDTH, WIDTH, -WIDTH - 1, WIDTH + 1 };

#define EDGE (WHITE | BLACK)
// clang-format off
#define STARTING_POS {\
    EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, \
    EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, \
    EDGE, 20,   18,   19,   21,   22,   19,   18,   20,   EDGE, \
    EDGE, 17,   17,   17,   17,   17,   17,   17,   17,   EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 33,   33,   33,   33,   33,   33,   33,   33,   EDGE, \
    EDGE, 36,   34,   35,   37,   38,   35,   34,   36,   EDGE, \
    EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, \
    EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, \
}
#define EMPTY_BOARD {\
    EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, \
    EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, 0,    0,    0,    0,    0,    0,    0,    0,    EDGE, \
    EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, \
    EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, EDGE, \
}
// lookup tables
constexpr int8_t ranks[120] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 2, 2, 2, 2, 2, 2, 2, 2, 0,
    0, 3, 3, 3, 3, 3, 3, 3, 3, 0,
    0, 4, 4, 4, 4, 4, 4, 4, 4, 0,
    0, 5, 5, 5, 5, 5, 5, 5, 5, 0,
    0, 6, 6, 6, 6, 6, 6, 6, 6, 0,
    0, 7, 7, 7, 7, 7, 7, 7, 7, 0,
    0, 8, 8, 8, 8, 8, 8, 8, 8, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0
}; 

constexpr int8_t files[120] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0,
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0,
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0,
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0,
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0,
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0,
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0,
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,0,0,0, 0
};
constexpr int8_t diagonals1[120] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 9, 8, 7, 6, 5, 4, 3, 2, 0,
    0, 10, 9, 8, 7, 6, 5, 4, 3, 0,
    0, 11, 10, 9, 8, 7, 6, 5, 4, 0,
    0, 12, 11, 10, 9, 8, 7, 6, 5, 0,
    0, 13, 12, 11, 10, 9, 8, 7,6, 0,
    0, 14, 13, 12, 11, 10, 9, 8, 7, 0,
    0, 15, 14, 13, 12, 11, 10, 9,8, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

}; 
constexpr int8_t diagonals2[120] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
    0, 2, 3, 4, 5, 6, 7, 8, 9, 0,
    0, 3, 4, 5, 6, 7, 8, 9, 10, 0,
    0, 4, 5, 6, 7, 8, 9, 10, 11, 0,
    0, 5, 6, 7, 8, 9, 10, 11, 12, 0,
    0, 6, 7, 8, 9, 10, 11, 12, 13, 0,
    0, 7, 8, 9, 10, 11, 12, 13, 14, 0,
    0, 8, 9, 10, 11, 12, 13, 14, 15, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// clang-format on
