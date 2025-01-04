#include "../../include/constants.h"
#include "../../src/game/game.h"
#include "../../src/game/renderer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_renderer() {
  Game game = {0};
  game.snake_length = GAME_INITIAL_SNAKE_LENGTH;
  game.state = GAME_STATE_RUNNING;
  for (int i = 0; i < GAME_INITIAL_SNAKE_LENGTH; i++) {
    game.snake[i] = (Position){5 - i, 5};
  }
  game.food = (Position){10, 5};

  char *output = render_game(&game);

  FILE *f = fopen(TESTS_HTML_OUTPUT_PATH, "w");
  if (f) {
    fprintf(f, "%s", output);
    fclose(f);
    printf("\nTest output written to %s\n", TESTS_HTML_OUTPUT_PATH);
  } else {
    printf("\nFailed to write test output\n");
  }

  assert(output != NULL);
  assert(strstr(output, "<!DOCTYPE html>") != NULL);
  assert(strstr(output, "cell snake") != NULL);
  assert(strstr(output, "cell food") != NULL);
  assert(strstr(output, "Score: 0") != NULL);
  assert(strstr(output, "State: Running") != NULL);

  printf("Renderer test passed!\n");
}

int main() {
  test_renderer();
  return 0;
}