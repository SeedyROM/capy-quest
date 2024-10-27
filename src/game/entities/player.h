#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "engine/gfx.h"
#include "game/behaviours/controllable.h"

typedef struct Player {
  Sprite sprite;
  Vec2 velocity;
  bool grounded;
} Player;

void PlayerInit(Player *player, Sprite *sprite);
void PlayerControl(Controllable *controllable, SDL_GameController *controller);
void PlayerUpdate(Player *player, f32 gravity);
