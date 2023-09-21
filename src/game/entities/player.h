#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "game/behaviours/controllable.h"
#include "std/gfx.h"

typedef struct Player
{
    Sprite sprite;
    Vec2 velocity;
    bool grounded;
} Player;

void PlayerInit(Player *player, Sprite *sprite);
void PlayerControl(Controllable *controllable, SDL_GameController *controller);
void PlayerUpdate(Player *player, f32 gravity);
void PlayerDraw(Player *player, SDL_Renderer *renderer);
