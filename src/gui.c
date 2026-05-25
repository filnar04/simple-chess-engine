#include "chess.h"
#include "ui.h"
#include "util.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_messagebox.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#define MIN(X, Y) (X < Y) ? X : Y
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define BOARD_START_X 0
#define BOARD_START_Y 0

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *textures[12] = { NULL };

int squareSize = MIN (WINDOW_WIDTH, WINDOW_HEIGHT) / 8;
#define TEXTURE_PATH "./gui/"

struct gamestate gameState;
int8_t playerColor = WHITE;

int
uiInit (int argc, char *argv[])
{
    if (!SDL_Init (SDL_INIT_VIDEO)) {
        SDL_Log ("Couldn't initialize SDL: %s", SDL_GetError ());
        return SDL_APP_FAILURE;
    }
    if (!SDL_CreateWindowAndRenderer ("test", WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_RESIZABLE, &window,
                                      &renderer)) {
        SDL_Log ("Couldn't create window or renderer: %s", SDL_GetError ());
        return SDL_APP_FAILURE;
    }

    SDL_Surface *surfaces[12] = { 0 };
    const char *png_names[12]
        = { "pawn_w", "knight_w", "bishop_w", "rook_w", "queen_w", "king_w",
            "pawn_b", "knight_b", "bishop_b", "rook_b", "queen_b", "king_b" };
    SDL_SetRenderLogicalPresentation (renderer, WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < 12; i++) {
        char *png_path = NULL;
        SDL_asprintf (
            &png_path, TEXTURE_PATH "%s.png",
            png_names[i]); /* allocate a string of the full file path */

        surfaces[i] = SDL_LoadPNG (png_path);
        if (!surfaces[i]) {
            SDL_Log ("Couldn't load png: %s", SDL_GetError ());
            return SDL_APP_FAILURE;
        }
        SDL_free (png_path);
        textures[i] = SDL_CreateTextureFromSurface (renderer, surfaces[i]);
        if (!textures[i]) {
            SDL_Log ("Couldn't create static texture: %s", SDL_GetError ());
            return SDL_APP_FAILURE;
        }
        SDL_DestroySurface (surfaces[i]);
    }

    playerColor = (time (0) % 2) ? WHITE : BLACK;
    return SDL_APP_CONTINUE;
}

int8_t check = 0;
int8_t highlightSquare = -1;

void
showBoard (uint8_t *board, uint8_t *highlight, uint8_t side)
{
    SDL_FRect rect;
    rect.h = squareSize;
    rect.w = squareSize;
    rect.x = BOARD_START_X;
    rect.y = BOARD_START_Y;
    SDL_SetRenderDrawColor (renderer, SQ_DARK, SDL_ALPHA_OPAQUE);
    SDL_RenderClear (renderer);
    int step, first, end;
    if (side == BLACK) {
        step = 1;
        first = 0;
        end = 8;
    } else {
        step = -1;
        first = 7;
        end = -1;
    }

    SDL_SetRenderDrawColor (renderer, SQ_LIGHT, SDL_ALPHA_OPAQUE);
    for (int r = first; r != end; r += step) {
        rect.x = BOARD_START_X;
        for (int f = 0; f < 8; f++) {
            int8_t square = 8 * r + f;
            int8_t piece = board[BOARD_TO_LOGIC (square)];
            if ((f + r) % 2) SDL_RenderFillRect (renderer, &rect);
            if (highlight[square]) {
                SDL_SetRenderDrawColor (renderer, 0x44, 0x44, 0x88, 0x80);
                SDL_RenderFillRect (renderer, &rect);
                SDL_SetRenderDrawColor (renderer, SQ_LIGHT, SDL_ALPHA_OPAQUE);
            }
            if (square == highlightSquare) {
                SDL_SetRenderDrawColor (renderer, 0x00, 0xff, 0x00,
                                        SDL_ALPHA_OPAQUE);
                SDL_RenderRect (renderer, &rect);
                SDL_SetRenderDrawColor (renderer, SQ_LIGHT, SDL_ALPHA_OPAQUE);
            }
            if (piece & PIECE_TYPE) {
                SDL_RenderTexture (renderer,
                                   textures[6 * ((piece & BLACK) > 0)
                                            + (piece & PIECE_TYPE) - 1],
                                   NULL, &rect);
            }
            rect.x += squareSize;
        }
        rect.y += squareSize;
    }
    SDL_RenderPresent (renderer);
}
void
handleEvent (SDL_Event e)
{
    if (e.type == SDL_EVENT_QUIT) {
        exit (EXIT_SUCCESS);
    }
    return;
}

uint8_t
pawnPromotion (int8_t squareCol, int8_t color)
{
    SDL_SetRenderDrawColor (renderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
    SDL_FRect background = { squareCol * squareSize, BOARD_START_Y, squareSize,
                             squareSize * 4 };
    SDL_FRect pieceSymbol
        = { squareCol * squareSize, BOARD_START_Y, squareSize, squareSize };
    SDL_RenderFillRect (renderer, &background);
    for (int i = QUEEN; i >= KNIGHT; i--) {
        SDL_RenderTexture (renderer, textures[6 * (color == BLACK) + i - 1],
                           NULL, &pieceSymbol);
        pieceSymbol.y += squareSize;
    }
    SDL_RenderPresent (renderer);
    SDL_Event e;
    while (1) {
        do {
            SDL_WaitEvent (&e);
            handleEvent (e);
        } while (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN);

        SDL_ConvertEventToRenderCoordinates (renderer, &e);
        switch ((int)e.button.y / squareSize) {
        case 0:
            return QUEEN;
        case 1:
            return ROOK;
        case 2:
            return BISHOP;
        case 3:
            return KNIGHT;
        default:
            return QUEEN;
        }
    }
}

int8_t
getClickedSquare (SDL_Event event, uint8_t side)
{
    SDL_ConvertEventToRenderCoordinates (renderer, &event);
    int squareRow = (event.button.y - BOARD_START_Y) / squareSize;
    int squareCol = (event.button.x - BOARD_START_X) / squareSize;
    if (squareRow >= 8 || squareCol >= 8 || squareRow < 0 || squareCol < 0)
        return -1;
    if (side == WHITE) squareRow = 7 - squareRow;
    int8_t square = 8 * squareRow + squareCol;
    return square;
}

uint8_t
uiSelectPiece (uint8_t *board, struct gamestate gameState, uint8_t moveArr[64])
{
    SDL_Event event;
    do {
        SDL_WaitEvent (&event);
        handleEvent (event);
    } while (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN);

    int8_t square = getClickedSquare (event, gameState.turn);
    if (square < 0) return 0;
    uint8_t logicSquare = BOARD_TO_LOGIC (square);
    uint8_t piece = board[logicSquare];

    if (!(piece & gameState.turn)) return 0;
    int8_t check = testCheck (
        board, gameState.turn == WHITE ? gameState.kingW : gameState.kingB,
        gameState.turn);
    int n = getMoveArray (board, gameState, logicSquare, check, moveArr);
    if (n == 0) return 0;
    return logicSquare;
}

uint8_t
uiMakeMove (uint8_t *board, struct gamestate *gameState, uint8_t *selected,
            uint8_t moveArr[64])
{
    SDL_Event event;
    do {
        SDL_WaitEvent (&event);
        handleEvent (event);
    } while (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN);

    int8_t square = getClickedSquare (event, gameState->turn);
    if (square < 0) {
        return 0;
        *selected = 0;
    }
    if (moveArr[square]) {
        struct move m;
        m.start = *selected;
        m.end = BOARD_TO_LOGIC (square);
        int8_t promRank
            = (playerColor == WHITE) ? WHITE_PROMOTION : BLACK_PROMOTION;
        int8_t p = 0;
        if (ranks[m.end] == promRank
            && (board[*selected] & PIECE_TYPE) == PAWN) {
            p = pawnPromotion (square % 8, gameState->turn);
        }
        m.promotion = p;
        makeMove (board, gameState, gameState->turn, m);
        return 1;
    }
    uint8_t logicSquare = BOARD_TO_LOGIC (square);
    if (board[logicSquare] & gameState->turn) {
        *selected = logicSquare;
    } else {
        *selected = 0;
    }
    return 0;
}

void
uiEnd (enum gameResult result)
{
    SDL_Event e;
    while (1) {
        SDL_WaitEvent (&e);
        if (e.type == SDL_EVENT_QUIT) return;
    }
}
