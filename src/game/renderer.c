#include "renderer.h"
#include "game.h"
#include <stdio.h>
#include <string.h>

void init_board(char board[GAME_BOARD_HEIGHT][GAME_BOARD_WIDTH]) {
  for (int y = 0; y < GAME_BOARD_HEIGHT; y++) {
    for (int x = 0; x < GAME_BOARD_WIDTH; x++) {
      board[y][x] = RENDERER_CELL_EMPTY;
    }
  }
}

void draw_snake(char board[GAME_BOARD_HEIGHT][GAME_BOARD_WIDTH], Game *game) {
  for (int i = 0; i < game->snake_length; i++) {
    Position pos = game->snake[i];
    board[pos.y][pos.x] = RENDERER_CELL_SNAKE;
  }
}

void draw_food(char board[GAME_BOARD_HEIGHT][GAME_BOARD_WIDTH], Game *game) {
  Position pos = game->food;
  board[pos.y][pos.x] = RENDERER_CELL_FOOD;
}

char *render_game(Game *game) {
  static char buffer[RENDERER_BUFFER_SIZE];
  static char board_html[RENDERER_BOARD_HTML_SIZE];
  static char info_html[RENDERER_INFO_HTML_SIZE];
  char board[GAME_BOARD_HEIGHT][GAME_BOARD_WIDTH];

  init_board(board);
  draw_snake(board, game);
  draw_food(board, game);

  char *ptr = board_html;
  *ptr = '\0';

  for (int y = 0; y < GAME_BOARD_HEIGHT; y++) {
    ptr += sprintf(ptr, "<div class='row'>");
    for (int x = 0; x < GAME_BOARD_WIDTH; x++) {
      const char *cell_class = board[y][x] == RENDERER_CELL_SNAKE ? "cell snake"
                               : board[y][x] == RENDERER_CELL_FOOD
                                   ? "cell food"
                                   : "cell empty";
      ptr += sprintf(ptr, "<div class='%s'></div>", cell_class);
    }
    ptr += sprintf(ptr, "</div>\n");
  }

  snprintf(info_html, RENDERER_INFO_HTML_SIZE,
           "<p>Score: %d</p><p>State: %s</p>",
           game->snake_length - GAME_INITIAL_SNAKE_LENGTH,
           game->state == GAME_STATE_RUNNING ? "Running"
           : game->state == GAME_STATE_OVER  ? "Game Over"
                                             : "Waiting");

  ptr = buffer;
  *ptr = '\0';

  FILE *template = fopen("src/game/template.html", "r");
  if (!template) {
    return "Error: Cannot open template file";
  }

  char line[RENDERER_INFO_HTML_SIZE];
  while (fgets(line, sizeof(line), template)) {
    if (strstr(line, "<!-- GAME_BOARD -->")) {
      ptr += sprintf(ptr, "%s", board_html);
    } else if (strstr(line, "<!-- GAME_INFO -->")) {
      ptr += sprintf(ptr, "%s", info_html);
    } else {
      ptr += sprintf(ptr, "%s", line);
    }
  }

  fclose(template);
  return buffer;
}