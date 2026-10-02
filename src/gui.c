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
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

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

extern uint8_t playerColor;
struct move player_move;

uint32_t GAMEEVENT;

static SDL_Thread *logic_thread;
static SDL_Semaphore *mov_sem;

static void uiLoop ();

void
uiInit (int argc, char *argv[])
{
    if (!SDL_Init (SDL_INIT_VIDEO)) {
        SDL_Log ("Couldn't initialize SDL: %s", SDL_GetError ());
        exit (EXIT_FAILURE);
    }
    if (!SDL_CreateWindowAndRenderer ("test", WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_RESIZABLE, &window,
                                      &renderer)) {
        SDL_Log ("Couldn't create window or renderer: %s", SDL_GetError ());
        exit (EXIT_FAILURE);
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
            exit (EXIT_FAILURE);
        }
        SDL_free (png_path);
        textures[i] = SDL_CreateTextureFromSurface (renderer, surfaces[i]);
        if (!textures[i]) {
            SDL_Log ("Couldn't create static texture: %s", SDL_GetError ());
            exit (EXIT_FAILURE);
        }
        SDL_DestroySurface (surfaces[i]);
    }

    SDL_SetEventEnabled (SDL_EVENT_MOUSE_MOTION, 0);

    GAMEEVENT = SDL_RegisterEvents (1);
    mov_sem = SDL_CreateSemaphore (0);

    logic_thread = SDL_CreateThread (gameLoop, "logic_thread", NULL);
    if (logic_thread == NULL) {
        puts (SDL_GetError ());
        exit (EXIT_FAILURE);
    }
    uiLoop ();
}

void
sendGameEvent (gameEventData *data)
{
    SDL_Event event;
    SDL_zero (event);
    event.type = GAMEEVENT;
    event.user.timestamp = SDL_GetTicksNS ();
    event.user.data1 = malloc (sizeof (gameEventData));
    memcpy (event.user.data1, data, sizeof (gameEventData));
    SDL_PushEvent (&event);
}

struct move
getPlayerMove ()
{
    SDL_WaitSemaphore (mov_sem);
    return player_move;
}

int8_t check = 0;
int8_t highlightSquare = -1;

static void
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
static void
handleEvent (SDL_Event e)
{
    if (e.type == SDL_EVENT_QUIT) {
        exit (EXIT_SUCCESS);
    }
    return;
}

static uint8_t
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
        } while (e.type != SDL_EVENT_MOUSE_BUTTON_DOWN);

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

static int8_t
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

static uint8_t
uiSelectPiece (SDL_Event clickEvent, uint8_t *board,
               struct gamestate gameState, uint8_t moveArr[64])
{
    int8_t square = getClickedSquare (clickEvent, gameState.turn);
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

#define SUCCESS 1
#define FAIL 0

static uint8_t
uiMakeMove (SDL_Event *event, uint8_t *board, struct gamestate *gameState,
            uint8_t *selected, uint8_t moveArr[64])
{
    int8_t square = getClickedSquare (*event, gameState->turn);
    if (square < 0) {
        *selected = 0;
        return FAIL;
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
        gameState->turn ^= COLOR_MASK;
        player_move = m;
        SDL_SignalSemaphore (mov_sem);
        *selected = 0;
        return SUCCESS;
    }
    uint8_t logicSquare = BOARD_TO_LOGIC (square);
    if (board[logicSquare] & gameState->turn) {
        *selected = logicSquare;
    } else {
        *selected = 0;
    }
    return FAIL;
}

static void
uiLoop ()
{
    uint8_t board[] = STARTING_POS;
    struct gamestate gameState
        = { WHITE, 25, 95, WHITE | BLACK, WHITE | BLACK, 0, 0 };
    uint8_t highlight[64] = { 0 };
    uint8_t inCheck = 0;
    uint8_t selected_square = 0;
    gameResult result;
    while (1) {
        showBoard (board, highlight, playerColor);
        SDL_Event e;
        SDL_WaitEvent (&e);
        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (gameState.turn != playerColor) continue;
            if (selected_square == 0) {
                selected_square
                    = uiSelectPiece (e, board, gameState, highlight);
            } else {
                uiMakeMove (&e, board, &gameState, &selected_square,
                            highlight);
                bzero (highlight, 64);
                if (selected_square) {
                    getMoveArray (board, gameState, selected_square, inCheck,
                                  highlight);
                }
            }
        } else if (e.type == GAMEEVENT) {
            gameEventData *event = e.user.data1;
            if (event->type == EVENT_MOVE) {
                makeMove (board, &gameState, gameState.turn, event->mov);
                gameState.turn ^= COLOR_MASK;
                inCheck = testCheck (board,
                                     playerColor == WHITE ? gameState.kingW
                                                          : gameState.kingB,
                                     playerColor);
            } else {
                result = event->res;
                break;
            }
        } else if (e.type == SDL_EVENT_QUIT)
            exit (EXIT_SUCCESS);
    }
    uiEnd (result);
}

void
uiStart ()
{
}

void
uiEnd (gameResult result)
{
    // placeholder
    switch (result) {
    case CHECKMATE_BLACK:
        puts ("Black won by checkmate.");
        break;
    case CHECKMATE_WHITE:
        puts ("White won by checkamte.");
        break;
    case DRAW_STALEMATE:
        puts ("The game ended in draw by stalemate.");
        break;
    case DRAW_50MOVE:
        puts ("The game ended in draw by 50 move rule.");
        break;
    case DRAW_REPETITION:
        puts ("The game ended in draw by repetition.");
        break;
    case DRAW_DEAD:
        puts ("The game ended in draw by insufficient material.");
        break;
    }
    SDL_Event e;
    while (1) {
        SDL_WaitEvent (&e);
        if (e.type == SDL_EVENT_QUIT) return;
    }
}
