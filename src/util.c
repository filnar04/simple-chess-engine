typedef signed char int8;

#define WIDTH 10
#define BOARD_SIZE 8
#define BOARD_MEM_SIZE 120
#define FIRST_SQUARE 21
#define LAST_SQUARE 99

#define HORIZONTAL_STEP 1
#define VERTICAL_STEP 10
#define KNIGHT_MOVES 8

enum pieces { PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING };
#define PIECE_TYPE 7
#define COLOR_MASK 060
#define WHITE 020
#define BLACK 040
#define HAS_MOVED 010

#define GAMESTATE_MEM_SIZE 8
#define KING_POS_W 0
#define KING_POS_B 1
#define SHORTCASTLE_W 2
#define SHORTCASTLE_B 3
#define LONGCASTLE_W 4
#define LONGCASTLE_B 5
#define ENPASSANT_POS 6
#define HALFMOVE_CLOCK 7

/*#define WHITE_PAWN_START 3*/
/*#define BLACK_PAWN_START 8*/
const int8 knightMoves[]
    = { -WIDTH + 2, -WIDTH - 2, -2 * WIDTH + 1, -2 * WIDTH - 1,
        WIDTH - 2,  WIDTH + 2,  2 * WIDTH + 1,  2 * WIDTH - 1 };
const int8 bishopMoves[] = { -WIDTH + 1, WIDTH - 1, -WIDTH - 1, WIDTH + 1 };
const int8 rookMoves[] = { -1, 1, -WIDTH, WIDTH };
const int8 kingMoves[]
    = { -1, 1, -WIDTH + 1, WIDTH - 1, -WIDTH, WIDTH, -WIDTH - 1, WIDTH + 1 };

#define EDGE WHITE | BLACK
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
// clang-format on
