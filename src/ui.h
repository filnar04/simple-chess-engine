#include "util.h"

void uiInit (int argc, char **argv);

struct uiInitData {
    int argc;
    char **argv;
    int sockfd;
};

typedef enum {
    CHECKMATE_WHITE,
    CHECKMATE_BLACK,
    DRAW_STALEMATE,
    DRAW_DEAD,
    DRAW_REPETITION,
    DRAW_50MOVE
} gameResult;

typedef enum {
    EVENT_MOVE,
    EVENT_END,
} gameEventType;

typedef struct {
    gameEventType type;
    union {
        struct move mov;
        gameResult res;
    };
} gameEventData;

int gameLoop (void *);
void sendGameEvent (gameEventData *d);
struct move getPlayerMove ();
void uiEnd (gameResult result);

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
