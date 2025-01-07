#include "logging.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

void log_message(const char *type, const char *msg, const char *color) {
  struct timeval tv;
  struct tm *tm_info;
  char time_str[16];

  gettimeofday(&tv, NULL);
  tm_info = localtime(&tv.tv_sec);

  strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
  snprintf(time_str + strlen(time_str), sizeof(time_str) - strlen(time_str),
           ".%03ld", tv.tv_usec / 1000);

  fprintf(stderr, "%s[%s] %s: %s%s\n", color, time_str, type, msg,
          ANSI_COLOR_RESET);
}

void log_message_info(const char *msg) {
  log_message("INFO", msg, ANSI_COLOR_GREEN);
}

void log_message_error(const char *msg) {
  log_message("ERROR", msg, ANSI_COLOR_RED);
}

void log_message_received(const char *msg) {
  log_message("RECEIVED", msg, ANSI_COLOR_BLUE);
}

void log_message_sending(const char *msg) {
  log_message("SENDING", msg, ANSI_COLOR_YELLOW);
}
