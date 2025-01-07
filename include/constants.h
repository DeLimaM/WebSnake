#ifndef CONSTANTS_H
#define CONSTANTS_H

#define GAME_BOARD_WIDTH 32
#define GAME_BOARD_HEIGHT 16
#define GAME_INITIAL_SNAKE_LENGTH GAME_BOARD_WIDTH / 4
#define GAME_TICK_RATE_MS 150

#define RENDERER_CELL_HTML_SIZE 48
#define RENDERER_ROW_HTML_SIZE 32
#define RENDERER_BOARD_HTML_SIZE                                               \
  ((GAME_BOARD_WIDTH * GAME_BOARD_HEIGHT * RENDERER_CELL_HTML_SIZE) +          \
   (GAME_BOARD_HEIGHT * RENDERER_ROW_HTML_SIZE))
#define RENDERER_BUFFER_SIZE (RENDERER_BOARD_HTML_SIZE + 8192)
#define RENDERER_TEMPLATE_CACHE_SIZE RENDERER_BUFFER_SIZE
#define RENDERER_INFO_HTML_SIZE 1024
#define RENDERER_TEMPLATE_PATH "src/game/template.html"
#define RENDERER_CELL_EMPTY ' '
#define RENDERER_CELL_SNAKE 'o'
#define RENDERER_CELL_FOOD 'x'

#define SERVER_PORT 8080
#define SERVER_HTTP_BUFFER_SIZE 4096
#define SERVER_SERVICE_INTERVAL 1
#define SERVER_LOG_BUFFER_SIZE 512
#define SERVER_TICK_RATE_MS 75

#define TESTS_HTML_OUTPUT_PATH "tests/out/test_output.html"

#endif // CONSTANTS_H