#ifndef LOGGING_H
#define LOGGING_H

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_CLEAR_LINE "\x1b[2K"
#define ANSI_COLOR_CURSOR_UP "\x1b[1A"

void log_message_info(const char *msg);
void log_message_error(const char *msg);
void log_message_received(const char *msg);
void log_message_sending(const char *msg);

#endif // LOGGING_H