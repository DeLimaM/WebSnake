#ifndef GAME_H
#define GAME_H

#include "../../include/constants.h"
#include "../../include/types.h"

void init_game(Game *game);
void update_game(Game *game, Direction new_direction);

#endif // GAME_H