#include "game.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void place_food(Game *game);
static int is_position_occupied(Game *game, Position pos);
static int check_collision(Game *game);

static void place_food(Game *game) {
  srand(time(NULL));
  do {
    game->food.x = rand() % GAME_BOARD_WIDTH;
    game->food.y = rand() % GAME_BOARD_HEIGHT;
  } while (is_position_occupied(game, game->food));
}

static int is_position_occupied(Game *game, Position pos) {
  for (int i = 0; i < game->snake_length; i++) {
    if (game->snake[i].x == pos.x && game->snake[i].y == pos.y) {
      return 1;
    }
  }
  return 0;
}

static int check_collision(Game *game) {
  Position head = game->snake[0];

  if (head.x < 0 || head.x >= GAME_BOARD_WIDTH || head.y < 0 ||
      head.y >= GAME_BOARD_HEIGHT) {
    return 1;
  }

  for (int i = 1; i < game->snake_length; i++) {
    if (head.x == game->snake[i].x && head.y == game->snake[i].y) {
      return 1;
    }
  }
  return 0;
}

void init_game(Game *game) {
  pthread_mutex_init(&game->mutex, NULL);
  game->snake_length = GAME_INITIAL_SNAKE_LENGTH;
  game->direction = DIRECTION_RIGHT;
  game->state = GAME_STATE_RUNNING;

  int start_x = GAME_BOARD_WIDTH / 2;
  int start_y = GAME_BOARD_HEIGHT / 2;

  for (int i = 0; i < GAME_INITIAL_SNAKE_LENGTH; i++) {
    game->snake[i].x = start_x - i;
    game->snake[i].y = start_y;
  }

  place_food(game);
}

int change_direction(Game *game, Direction new_direction) {
  pthread_mutex_lock(&game->mutex);
  if ((new_direction == DIRECTION_LEFT && game->direction != DIRECTION_RIGHT) ||
      (new_direction == DIRECTION_RIGHT && game->direction != DIRECTION_LEFT) ||
      (new_direction == DIRECTION_UP && game->direction != DIRECTION_DOWN) ||
      (new_direction == DIRECTION_DOWN && game->direction != DIRECTION_UP)) {
    game->direction = new_direction;
  }
  pthread_mutex_unlock(&game->mutex);
  return 0;
}

void update_game(Game *game) {
  pthread_mutex_lock(&game->mutex);

  if (game->state != GAME_STATE_RUNNING) {
    pthread_mutex_unlock(&game->mutex);
    return;
  }

  Position prev_positions[GAME_BOARD_WIDTH * GAME_BOARD_HEIGHT];
  memcpy(prev_positions, game->snake, sizeof(Position) * game->snake_length);

  switch (game->direction) {
  case DIRECTION_UP:
    game->snake[0].y--;
    break;
  case DIRECTION_DOWN:
    game->snake[0].y++;
    break;
  case DIRECTION_LEFT:
    game->snake[0].x--;
    break;
  case DIRECTION_RIGHT:
    game->snake[0].x++;
    break;
  }

  if (check_collision(game)) {
    game->state = GAME_STATE_OVER;
    pthread_mutex_unlock(&game->mutex);
    return;
  }

  if (game->snake[0].x == game->food.x && game->snake[0].y == game->food.y) {
    game->snake_length++;
    place_food(game);
  }

  for (int i = 1; i < game->snake_length; i++) {
    game->snake[i] = prev_positions[i - 1];
  }

  pthread_mutex_unlock(&game->mutex);
}

void cleanup_game(Game *game) { pthread_mutex_destroy(&game->mutex); }