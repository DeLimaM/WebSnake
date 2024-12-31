#ifndef GAME_H
#define GAME_H

#include "../../include/constants.h"
#include <pthread.h>

typedef struct {
  Position snake[BOARD_WIDTH * BOARD_HEIGHT];
  int snake_length;
  Position food;
  Direction direction;
  GameState state;
  pthread_mutex_t mutex;
} Game;

void init_game(Game *game);
void update_game(Game *game);
int change_direction(Game *game, Direction new_direction);

#endif // GAME_H