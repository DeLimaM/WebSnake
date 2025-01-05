#include "ws_server.h"
#include <json-c/json.h>
#include <libwebsockets.h>
#include <string.h>

static int callback_snake(struct lws *wsi, enum lws_callback_reasons reason,
                          void *user, void *in, size_t len);

static ServerState server_state;
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

static void log_message(const char *type, const char *msg, const char *color) {
  time_t now;
  time(&now);
  char *date = ctime(&now);
  date[strlen(date) - 1] = '\0';
  fprintf(stderr, "%s[%s] %s: %s%s\n", color, date, type, msg,
          ANSI_COLOR_RESET);
}

static int handle_http_request(struct lws *wsi) {
  if (lws_http_transaction_completed(wsi))
    return 0;

  char *rendered = render_game(&server_state.game);
  if (!rendered) {
    return -1;
  }

  unsigned char buffer[LWS_PRE + SERVER_HTTP_BUFFER_SIZE];
  unsigned char *p = &buffer[LWS_PRE];
  if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p,
                                 buffer + sizeof(buffer)) < 0) {
    return -1;
  }
  if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                   (unsigned char *)"text/html", 9, &p,
                                   buffer + sizeof(buffer)) < 0) {
    return -1;
  }
  if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_LENGTH,
                                   (unsigned char *)&rendered, strlen(rendered),
                                   &p, buffer + sizeof(buffer)) < 0) {
    return -1;
  }
  if (lws_finalize_http_header(wsi, &p, buffer + sizeof(buffer)) < 0) {
    return -1;
  }

  lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE),
            LWS_WRITE_HTTP_HEADERS);

  lws_write(wsi, (unsigned char *)rendered, strlen(rendered), LWS_WRITE_HTTP);

  return 0;
}

static struct json_object *parse_json_message(const char *message) {
  struct json_object *json = json_tokener_parse(message);
  if (!json) {
    log_message("ERROR", "Failed to parse JSON", ANSI_COLOR_RED);
    return NULL;
  }
  return json;
}

static Direction parse_direction_string(const char *dir_str) {
  if (strcmp(dir_str, "up") == 0)
    return DIRECTION_UP;
  if (strcmp(dir_str, "down") == 0)
    return DIRECTION_DOWN;
  if (strcmp(dir_str, "left") == 0)
    return DIRECTION_LEFT;
  if (strcmp(dir_str, "right") == 0)
    return DIRECTION_RIGHT;
  else {
    log_message("ERROR", "Invalid direction", ANSI_COLOR_RED);
    return DIRECTION_NONE;
  }
}

static int callback_snake(struct lws *wsi, enum lws_callback_reasons reason,
                          void *user __attribute__((unused)), void *in,
                          size_t len __attribute__((unused))) {
  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED: {
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
  }

  case LWS_CALLBACK_CLOSED: {
    log_message("DISCONNECT", "Client disconnected", ANSI_COLOR_RED);
    break;
  }

  case LWS_CALLBACK_RECEIVE: {
    log_message("RECEIVED", (char *)in, ANSI_COLOR_BLUE);

    struct json_object *json = parse_json_message((char *)in);
    if (!json) {
      break;
    }

    struct json_object *direction_obj;
    if (json_object_object_get_ex(json, "direction", &direction_obj)) {
      const char *dir_str = json_object_get_string(direction_obj);
      Direction new_dir = parse_direction_string(dir_str);

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

  log_message("INFO", "Server started", ANSI_COLOR_GREEN);

  while (1) {
    lws_service(server_state.context, 50);
  }

  log_message("INFO", "Server stopped", ANSI_COLOR_RED);

  return 0;
}