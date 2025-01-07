#include "ws_server.h"
#include <json-c/json.h>
#include <libwebsockets.h>
#include <string.h>

static int callback_snake(struct lws *wsi, enum lws_callback_reasons reason,
                          void *user, void *in, size_t len);

static ServerState server_state;
static struct lws_protocols protocols[] = {
    {
        .name = "snake-protocol",
        .callback = callback_snake,
        .per_session_data_size = 0,
        .rx_buffer_size = 4096,
        .id = 1,
    },
    {
        .name = "http",
        .callback = callback_snake,
        .per_session_data_size = 0,
        .rx_buffer_size = SERVER_HTTP_BUFFER_SIZE,
        .id = 2,
    }};

static int add_http_headers(struct lws *wsi, unsigned char **p,
                            unsigned char *end, size_t content_length) {
  return lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "text/html",
                                     content_length, p, end) ||
         lws_add_http_header_by_token(wsi, WSI_TOKEN_CONNECTION,
                                      (unsigned char *)"Upgrade", 7, p, end) ||
         lws_add_http_header_by_token(
             wsi, WSI_TOKEN_UPGRADE, (unsigned char *)"websocket", 9, p, end) ||
         lws_finalize_http_header(wsi, p, end);
}

static int write_http_response(struct lws *wsi, unsigned char *buffer,
                               unsigned char *p, const char *rendered) {
  if (lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE),
                LWS_WRITE_HTTP_HEADERS) < 0) {
    log_message_error("Failed to write HTTP headers");
    return -1;
  }

  if (lws_write(wsi, (unsigned char *)rendered, strlen(rendered),
                LWS_WRITE_HTTP_FINAL) < 0) {
    log_message_error("Failed to write HTTP content");
    return -1;
  }

  log_message_sending("Initial HTML render");
  return lws_http_transaction_completed(wsi);
}

static void reset_timer(struct lws *wsi) {
  lws_set_timer_usecs(wsi, SERVER_TICK_RATE_MS * 1000);
}

static void send_game_update(struct lws *wsi) {
  char *rendered = render_game(&server_state.game);
  if (!rendered) {
    log_message_error("Failed to render game");
    return;
  }

  log_message_sending("Updated game state");
  lws_write(wsi, (unsigned char *)rendered, strlen(rendered), LWS_WRITE_TEXT);
  reset_timer(wsi);
}

static struct json_object *parse_json_message(const char *message) {
  struct json_object *json = json_tokener_parse(message);
  if (!json) {
    char error_buf[SERVER_LOG_BUFFER_SIZE];
    snprintf(error_buf, sizeof(error_buf), "Failed to parse JSON message: %s",
             message);
    log_message_error(error_buf);
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
    log_message_error("Invalid direction string");
    return DIRECTION_NONE;
  }
}

static void handle_ws_message(struct lws *wsi, const char *message) {
  struct json_object *json = parse_json_message(message);
  if (!json) {
    return;
  }

  struct json_object *direction_obj;
  if (json_object_object_get_ex(json, "direction", &direction_obj)) {
    const char *dir_str = json_object_get_string(direction_obj);
    Direction new_direction = parse_direction_string(dir_str);
    update_game(&server_state.game, new_direction);
    send_game_update(wsi);
  }
  json_object_put(json);
}

static int handle_http_request(struct lws *wsi) {
  log_message_received("HTTP request");
  char *rendered = render_game(&server_state.game);
  if (!rendered) {
    log_message_error("Failed to render game");
    return -1;
  }

  unsigned char buffer[LWS_PRE + SERVER_HTTP_BUFFER_SIZE];
  unsigned char *p = &buffer[LWS_PRE];
  unsigned char *end = buffer + sizeof(buffer);
  size_t content_length = strlen(rendered);

  if (add_http_headers(wsi, &p, end, content_length)) {
    log_message_error("Failed to add HTTP headers");
    return -1;
  }

  return write_http_response(wsi, buffer, p, rendered);
}

static int handle_ws_established(struct lws *wsi) {
  log_message_info("New client connected");
  pthread_mutex_lock(&server_state.game.mutex);
  init_game(&server_state.game);
  pthread_mutex_unlock(&server_state.game.mutex);
  log_message_info("Game initialized");

  update_game(&server_state.game, server_state.game.direction);
  send_game_update(wsi);
  reset_timer(wsi);
  return 0;
}

static int handle_ws_closed(void) {
  log_message_info("Client disconnected");
  return 0;
}

static int handle_ws_receive(struct lws *wsi, const char *message) {
  log_message_received(message);
  handle_ws_message(wsi, message);
  return 0;
}

static int handle_timer_callback(struct lws *wsi) {
  update_game(&server_state.game, server_state.game.direction);
  send_game_update(wsi);
  return 0;
}

static int callback_snake(struct lws *wsi, enum lws_callback_reasons reason,
                          void *user __attribute__((unused)), void *in,
                          size_t len __attribute__((unused))) {
  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED:
    return handle_ws_established(wsi);

  case LWS_CALLBACK_CLOSED:
    return handle_ws_closed();

  case LWS_CALLBACK_RECEIVE:
    return handle_ws_receive(wsi, (char *)in);

  case LWS_CALLBACK_HTTP:
    return handle_http_request(wsi);

  case LWS_CALLBACK_TIMER:
    return handle_timer_callback(wsi);

  default:
    break;
  }
  return 0;
}

// TODO : serve the js and css files with the correct mime type
int start_ws_server(int port) {
  struct lws_context_creation_info info = {
      .port = port,
      .protocols = protocols,
      .gid = -1,
      .uid = -1,
      .options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT,
  };

  init_game(&server_state.game);

  server_state.context = lws_create_context(&info);
  if (!server_state.context) {
    log_message_error("Failed to create server context");
    return -1;
  }
  log_message_info("Server started");

  while (1) {
    lws_service(server_state.context, SERVER_SERVICE_INTERVAL);
  }
  log_message_info("Server stopped");

  return 0;
}