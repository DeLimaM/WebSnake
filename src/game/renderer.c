#include "renderer.h"
#include "game.h"
#include <stdio.h>
#include <string.h>

#define CELL_EMPTY ' '
#define CELL_SNAKE 'o'
#define CELL_FOOD 'x'

void init_board(char board[BOARD_WIDTH][BOARD_HEIGHT]) {
  for (int i = 0; i < BOARD_WIDTH; i++) {
    for (int j = 0; j < BOARD_HEIGHT; j++) {
      board[i][j] = CELL_EMPTY;
    }
  }
}

void draw_snake(char board[BOARD_WIDTH][BOARD_HEIGHT], Game *game) {
  for (int i = 0; i < game->snake_length; i++) {
    Position pos = game->snake[i];
    board[pos.x][pos.y] = CELL_SNAKE;
  }
}

void draw_food(char board[BOARD_WIDTH][BOARD_HEIGHT], Game *game) {
  Position pos = game->food;
  board[pos.x][pos.y] = CELL_FOOD;
}

void append_game_infos(char *buffer, Game *game) {
  char infos[128];
  sprintf(infos, "\nScore: %d | State: %s\n",
          game->snake_length - INITIAL_SNAKE_LENGTH,
          game->state == RUNNING     ? "Running"
          : game->state == GAME_OVER ? "Game Over"
                                     : "Waiting");
  strcat(buffer, infos);
}

char *render_game(Game *game) {
  static char buffer[BOARD_WIDTH * BOARD_HEIGHT];

  char board[BOARD_WIDTH][BOARD_HEIGHT];
  init_board(board);

  draw_snake(board, game);
  draw_food(board, game);

  char *ptr = buffer;
  ptr += sprintf(ptr, "+%.*s+\n", BOARD_WIDTH, "----------------");

  for (int j = 0; j < BOARD_HEIGHT; j++) {
    ptr += sprintf(ptr, "|");
    for (int i = 0; i < BOARD_WIDTH; i++) {
      ptr += sprintf(ptr, "%c", board[i][j]);
    }
    ptr += sprintf(ptr, "|\n");
  }

  ptr += sprintf(ptr, "+%.*s+\n", BOARD_WIDTH, "----------------");
  append_game_infos(buffer, game);

  return buffer;

  append_game_infos(buffer, game);
}