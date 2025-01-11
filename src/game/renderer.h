#ifndef RENDERER_H
#define RENDERER_H

#include "../../include/constants.h"
#include "../../include/types.h"
#include "../logging/logging.h"
#include "game.h"

char *render_game(Game *game);
char *render_game_state_json(Game *game);

#endif // RENDERER_H