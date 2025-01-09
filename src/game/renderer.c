#include "renderer.h"
#include <stdio.h>
#include <string.h>

static char template_cache[RENDERER_TEMPLATE_CACHE_SIZE];
static int template_loaded = 0;

static int load_template() {
  if (template_loaded) {
    return 1;
  }

  FILE *template = fopen(RENDERER_TEMPLATE_PATH, "r");
  if (!template) {
    return 0;
  }

  size_t bytes_read =
      fread(template_cache, 1, RENDERER_BUFFER_SIZE - 1, template);
  template_cache[bytes_read] = '\0';
  template_loaded = 1;

  fclose(template);
  return 1;
}

static void generate_info_html(Game *game, char *info_html) {
  snprintf(info_html, RENDERER_INFO_HTML_SIZE,
           "<p>Score: %d</p><p>State: %s</p>",
           game->snake_length - GAME_INITIAL_SNAKE_LENGTH,
           game->state == GAME_STATE_RUNNING ? "Running"
           : game->state == GAME_STATE_OVER  ? "Game Over"
                                             : "Waiting");
}

static char *generate_board_html(Game *game, char *buffer) {
  char *ptr = buffer;
  *ptr = '\0';

  for (int y = 0; y < GAME_BOARD_HEIGHT; y++) {
    ptr += sprintf(ptr, "<div class='row'>");
    for (int x = 0; x < GAME_BOARD_WIDTH; x++) {
      const char *cell_class = "cell empty";
      int snake_index = -1;

      for (int i = 0; i < game->snake_length; i++) {
        if (game->snake[i].x == x && game->snake[i].y == y) {
          cell_class = "cell snake";
          snake_index = i;
          break;
        }
      }

      if (game->food.x == x && game->food.y == y) {
        cell_class = "cell food";
        ptr += sprintf(ptr, "<div class='%s'></div>", cell_class);
      } else if (snake_index >= 0) {
        ptr += sprintf(ptr, "<div class='%s' data-index='%d'></div>",
                       cell_class, snake_index);
      } else {
        ptr += sprintf(ptr, "<div class='%s'></div>", cell_class);
      }
    }
    ptr += sprintf(ptr, "</div>\n");
  }
  return ptr;
}

static void replace_template_markers(char *buffer, const char *board_html,
                                     const char *info_html) {
  char *ptr = buffer;
  *ptr = '\0';
  char *template_ptr = template_cache;

  while (*template_ptr) {
    if (strstr(template_ptr, "<!-- GAME_BOARD -->") == template_ptr) {
      ptr += sprintf(ptr, "%s", board_html);
      template_ptr += strlen("<!-- GAME_BOARD -->");
    } else if (strstr(template_ptr, "<!-- GAME_INFO -->") == template_ptr) {
      ptr += sprintf(ptr, "%s", info_html);
      template_ptr += strlen("<!-- GAME_INFO -->");
    } else {
      *ptr++ = *template_ptr++;
    }
  }
  *ptr = '\0';
}

char *render_game(Game *game) {
  static char buffer[RENDERER_BUFFER_SIZE];
  static char board_html[RENDERER_BOARD_HTML_SIZE];
  static char info_html[RENDERER_INFO_HTML_SIZE];

  if (!load_template()) {
    log_message_error("The renderer failed to load the template");
    return NULL;
  }

  generate_board_html(game, board_html);
  generate_info_html(game, info_html);
  replace_template_markers(buffer, board_html, info_html);

  return buffer;
}