#pragma once

#include "util.h"
#include <stdint.h>
#include <stdlib.h>
struct move getBestMove (uint8_t *board, struct gamestate *state,
                         uint maxdepth);

int eval (uint8_t *board, struct gamestate *state);
