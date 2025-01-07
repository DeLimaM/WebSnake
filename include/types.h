#ifndef TYPES_H
#define TYPES_H

#include <libwebsockets.h>
#include <pthread.h>

typedef enum {
  GAME_STATE_WAITING,
  GAME_STATE_RUNNING,
  GAME_STATE_OVER
} GameState;
typedef enum {
  DIRECTION_UP,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
  DIRECTION_RIGHT,
  DIRECTION_NONE
} Direction;
typedef struct {
  int x;
  int y;
} Position;
typedef struct {
  Position snake[GAME_BOARD_WIDTH * GAME_BOARD_HEIGHT];
  int snake_length;
  Position food;
  Direction direction;
  GameState state;
  pthread_mutex_t mutex;
  struct timespec last_update;
} Game;

typedef struct {
  Game game;
  struct lws_context *context;
} ServerState;

#endif // TYPES_H
