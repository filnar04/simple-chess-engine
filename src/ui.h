#include "util.h"

void showBoard (uint8_t *board, uint8_t *highlight, uint8_t side);
int uiInit (int argc, char **argv);
uint8_t uiSelectPiece (uint8_t *board, struct gamestate gameState,
                       uint8_t moveArr[64]);
uint8_t uiMakeMove (uint8_t *board, struct gamestate *gameState,
                    uint8_t *selected, uint8_t moveArr[64]);

enum gameResult {
    CHECKMATE_WHITE,
    CHECKMATE_BLACK,
    DRAW_STALEMATE,
    DRAW_DEAD,
    DRAW_REPETITION,
    DRAW_50MOVE
};
void uiEnd (enum gameResult result);

#define SQ_LIGHT_R 192
#define SQ_LIGHT_G 156
#define SQ_LIGHT_B 128
#define SQ_DARK_R 128
#define SQ_DARK_G 92
#define SQ_DARK_B 64

#define SQ_LIGHT SQ_LIGHT_R, SQ_LIGHT_G, SQ_LIGHT_B
#define SQ_DARK SQ_DARK_R, SQ_DARK_G, SQ_DARK_B

#define SQUARE_FROM_STR(_str) _str[0] - 'a' + (_str[1] - '1') * 8
#define LOGICAL_FROM_STR(_str) 21 + _str[0] - 'a' + (_str[1] - '1') * 10
