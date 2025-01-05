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

static int handle_http_request(struct lws *wsi) {
  if (lws_http_transaction_completed(wsi)) {
    return 0;
  }

  char *rendered = render_game(&server_state.game);
  if (!rendered) {
    log_message_error("Failed to render game");
    return -1;
  }

  size_t content_length = strlen(rendered);

  unsigned char buffer[LWS_PRE + SERVER_HTTP_BUFFER_SIZE];
  unsigned char *p = &buffer[LWS_PRE];
  unsigned char *end = buffer + sizeof(buffer);

  if (lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "text/html",
                                  content_length, &p, end) ||
      lws_add_http_header_by_token(wsi, WSI_TOKEN_CONNECTION,
                                   (unsigned char *)"Upgrade", 7, &p, end) ||
      lws_add_http_header_by_token(wsi, WSI_TOKEN_UPGRADE,
                                   (unsigned char *)"websocket", 9, &p, end) ||
      lws_finalize_http_header(wsi, &p, end)) {
    return -1;
  }

  if (lws_write(wsi, buffer + LWS_PRE, p - (buffer + LWS_PRE),
                LWS_WRITE_HTTP_HEADERS) < 0) {
    return -1;
  }

  if (lws_write(wsi, (unsigned char *)rendered, content_length,
                LWS_WRITE_HTTP) < 0) {
    return -1;
  }

  log_message_sending("Initial HTML render");
  if (lws_write(wsi, (unsigned char *)rendered, strlen(rendered),
                LWS_WRITE_HTTP) < 0) {
    return -1;
  }

  return lws_http_transaction_completed(wsi);
}

static struct json_object *parse_json_message(const char *message) {
  struct json_object *json = json_tokener_parse(message);
  if (!json) {
    log_message_error("Failed to parse JSON message");
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

static int callback_snake(struct lws *wsi, enum lws_callback_reasons reason,
                          void *user __attribute__((unused)), void *in,
                          size_t len __attribute__((unused))) {
  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED: {
    log_message_info("New client connected");
    pthread_mutex_lock(&server_state.game.mutex);
    init_game(&server_state.game);
    pthread_mutex_unlock(&server_state.game.mutex);
    log_message_info("Game initialized");

    char *rendered = render_game(&server_state.game);
    if (rendered) {
      lws_write(wsi, (unsigned char *)rendered, strlen(rendered),
                LWS_WRITE_TEXT);
      log_message_sending("Updated game state");
    }
    break;
  }

  case LWS_CALLBACK_CLOSED: {
    log_message_info("Client disconnected");
    break;
  }

  case LWS_CALLBACK_RECEIVE: {
    log_message_received((char *)in);

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
      log_message_error("Failed to render game");
      break;
    }

    log_message_sending("Updated game state");
    lws_write(wsi, (unsigned char *)rendered, strlen(rendered), LWS_WRITE_TEXT);
    break;
  }

  case LWS_CALLBACK_HTTP: {
    log_message_received("HTTP request");
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
  log_message_info("Server started");

  while (1) {
    lws_service(server_state.context, SERVER_SERVICE_INTERVAL);
  }
  log_message_info("Server stopped");

  return 0;
}