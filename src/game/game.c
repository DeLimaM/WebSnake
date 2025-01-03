#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void place_food(Game *game);
static int is_position_occupied(Game *game, Position pos);
static int check_collision(Game *game);

static void place_food(Game *game) {
  srand(time(NULL));
  do {
    game->food.x = rand() % BOARD_WIDTH;
    game->food.y = rand() % BOARD_HEIGHT;
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

  if (head.x < 0 || head.x >= BOARD_WIDTH || head.y < 0 ||
      head.y >= BOARD_HEIGHT) {
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
  game->snake_length = 1;
  game->direction = RIGHT;
  game->state = RUNNING;

  game->snake[0].x = BOARD_WIDTH / 2;
  game->snake[0].y = BOARD_HEIGHT / 2;

  place_food(game);
}

int change_direction(Game *game, Direction new_direction) {
  pthread_mutex_lock(&game->mutex);
  if ((new_direction == LEFT && game->direction != RIGHT) ||
      (new_direction == RIGHT && game->direction != LEFT) ||
      (new_direction == UP && game->direction != DOWN) ||
      (new_direction == DOWN && game->direction != UP)) {
    game->direction = new_direction;
  }
  pthread_mutex_unlock(&game->mutex);
  return 0;
}

void update_game(Game *game) {
  pthread_mutex_lock(&game->mutex);

  if (game->state != RUNNING) {
    pthread_mutex_unlock(&game->mutex);
    return;
  }

  Position prev_positions[BOARD_WIDTH * BOARD_HEIGHT];
  memcpy(prev_positions, game->snake, sizeof(Position) * game->snake_length);

  switch (game->direction) {
  case UP:
    game->snake[0].y--;
    break;
  case DOWN:
    game->snake[0].y++;
    break;
  case LEFT:
    game->snake[0].x--;
    break;
  case RIGHT:
    game->snake[0].x++;
    break;
  }

  if (check_collision(game)) {
    game->state = GAME_OVER;
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