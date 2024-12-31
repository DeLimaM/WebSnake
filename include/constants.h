#ifndef CONSTANTS_H
#define CONSTANTS_H

#define BOARD_WIDTH 20
#define BOARD_HEIGHT 20
#define INITIAL_SNAKE_LENGTH 4

typedef enum { WAITING, RUNNING, GAME_OVER } GameState;

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct {
  int x;
  int y;
} Position;

#endif // CONSTANTS_H