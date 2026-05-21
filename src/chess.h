#include "util.h"

unsigned int searchMoves (uint8_t *board, struct gamestate gameState,
                          struct move *moveList);

int8_t testCheck (uint8_t *board, int8_t pos, int8_t color);

void makeMove (uint8_t *board, struct gamestate *gameState, int8_t color,
               struct move m);

unsigned int searchMoves (uint8_t *board, struct gamestate gameState,
                          struct move *moveList);

unsigned int findPieceMoves (uint8_t *board, struct gamestate gameState,
                             int8_t square, int8_t inCheck,
                             struct move *moveList);
