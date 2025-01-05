#include "ws_server.h"
#include <json-c/json.h>
#include <libwebsockets.h>
#include <string.h>

static ServerState server_state;

static void log_message(const char *type, const char *msg, const char *color) {
  time_t now;
  time(&now);
  char *date = ctime(&now);
  date[strlen(date) - 1] = '\0';
  fprintf(stderr, "%s[%s] %s: %s%s\n", color, date, type, msg,
          ANSI_COLOR_RESET);
}

static int handle_http_request(struct lws *wsi) {
  char *rendered = render_game(&server_state.game);
  if (!rendered) {
    return -1;
  }

  unsigned char buffer[LWS_PRE + SERVER_HTTP_BUFFER_SIZE];
  unsigned char *p = &buffer[LWS_PRE];
  if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p,
                                 buffer + sizeof(buffer)))
    return 1;
  if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                   (unsigned char *)"text/html", 9, &p,
                                   buffer + sizeof(buffer)))
    return 1;
  if (lws_finalize_http_header(wsi, &p, buffer + sizeof(buffer)))
    return 1;

  log_message("SENDING", "HTTP", ANSI_COLOR_GREEN);

  lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE),
            LWS_WRITE_HTTP_HEADERS);

  lws_write(wsi, (unsigned char *)rendered, strlen(rendered), LWS_WRITE_HTTP);
  return lws_http_transaction_completed(wsi);
}

static int callback_snake(struct lws *wsi, enum lws_callback_reasons reason,
                          void *user __attribute__((unused)), void *in,
                          size_t len __attribute__((unused))) {
  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED:
    log_message("CONNECT", "New client connected", ANSI_COLOR_GREEN);
    pthread_mutex_lock(&server_state.game.mutex);
    init_game(&server_state.game);
    pthread_mutex_unlock(&server_state.game.mutex);

    char *rendered = render_game(&server_state.game);
    if (rendered) {
      lws_write(wsi, (unsigned char *)rendered, strlen(rendered),
                LWS_WRITE_TEXT);
    }
    break;

  case LWS_CALLBACK_CLOSED:
    log_message("DISCONNECT", "Client disconnected", ANSI_COLOR_RED);
    break;

  case LWS_CALLBACK_RECEIVE: {
    log_message("RECEIVED", (char *)in, ANSI_COLOR_BLUE);

    struct json_object *json = json_tokener_parse((char *)in);
    if (!json) {
      log_message("ERROR", "Failed to parse JSON", ANSI_COLOR_RED);
      break;
    }

    struct json_object *direction_obj;
    if (json_object_object_get_ex(json, "direction", &direction_obj)) {
      const char *dir_str = json_object_get_string(direction_obj);
      Direction new_dir;

      if (strcmp(dir_str, "up") == 0)
        new_dir = DIRECTION_UP;
      else if (strcmp(dir_str, "down") == 0)
        new_dir = DIRECTION_DOWN;
      else if (strcmp(dir_str, "left") == 0)
        new_dir = DIRECTION_LEFT;
      else if (strcmp(dir_str, "right") == 0)
        new_dir = DIRECTION_RIGHT;

      change_direction(&server_state.game, new_dir);
      update_game(&server_state.game);
    }
    json_object_put(json);

    char *rendered = render_game(&server_state.game);
    if (!rendered) {
      log_message("ERROR", "Failed to render game", ANSI_COLOR_RED);
      break;
    }

    log_message("SENDING", "HTML", ANSI_COLOR_GREEN);
    lws_write(wsi, (unsigned char *)rendered, strlen(rendered), LWS_WRITE_TEXT);
    break;
  }

  case LWS_CALLBACK_HTTP: {
    log_message("HTTP", "HTTP request received", ANSI_COLOR_BLUE);
    return handle_http_request(wsi);
  }

  default:
    break;
  }
  return 0;
}

static struct lws_protocols protocols[] = {{.name = "http",
                                            .callback = callback_snake,
                                            .per_session_data_size = 0,
                                            .rx_buffer_size = 0},
                                           {.name = "snake-protocol",
                                            .callback = callback_snake,
                                            .per_session_data_size = 0,
                                            .rx_buffer_size = 4096,
                                            .id = 1,
                                            .user = NULL},
                                           {.name = NULL,
                                            .callback = NULL,
                                            .per_session_data_size = 0,
                                            .rx_buffer_size = 0,
                                            .id = 0,
                                            .user = NULL}};

int start_ws_server(int port) {
  struct lws_context_creation_info info = {
      .port = port,
      .protocols = protocols,
      .gid = -1,
      .uid = -1,
  };

  init_game(&server_state.game);

  server_state.context = lws_create_context(&info);
  if (!server_state.context) {
    return -1;
  }

  while (1) {
    lws_service(server_state.context, 50);
  }

  return 0;
}