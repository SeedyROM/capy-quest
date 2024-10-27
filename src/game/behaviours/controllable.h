#pragma once

#include <SDL2/SDL.h>

#include "engine/util.h"

typedef struct Controllable {
  Vec2 *position;
  Vec2 *velocity;
  bool *grounded;

  void (*update)(struct Controllable *controllable,
                 SDL_GameController *controller);
} Controllable;

void ControllableInit(Controllable *controllable, Vec2 *position,
                      Vec2 *velocity, bool *grounded,
                      void (*update)(Controllable *controllable,
                                     SDL_GameController *controller));
void ControllableUpdate(Controllable *controllable,
                        SDL_GameController *controller);
