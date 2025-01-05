#include "../include/constants.h"
#include "server/ws_server.h"

int main(int argc __attribute__((unused)),
         char *argv[] __attribute__((unused))) {
  start_ws_server(SERVER_PORT);
  return 0;
}