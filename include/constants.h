#ifndef CONSTANTS_H
#define CONSTANTS_H

#define RENDERER_BUFFER_SIZE 4096
#define RENDERER_BOARD_HTML_SIZE 2048
#define RENDERER_INFO_HTML_SIZE 256
#define RENDERER_TEMPLATE_PATH "src/game/template.html"
#define RENDERER_CELL_EMPTY ' '
#define RENDERER_CELL_SNAKE 'o'
#define RENDERER_CELL_FOOD 'x'

#define GAME_BOARD_WIDTH 20
#define GAME_BOARD_HEIGHT 10
#define GAME_INITIAL_SNAKE_LENGTH 5

typedef enum {
  GAME_STATE_WAITING,
  GAME_STATE_RUNNING,
  GAME_STATE_OVER
} GameState;

typedef enum {
  DIRECTION_UP,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
  DIRECTION_RIGHT
} Direction;

typedef struct {
  int x;
  int y;
} Position;

#define TESTS_HTML_OUTPUT_PATH "tests/out/test_output.html"

#endif // CONSTANTS_H