#pragma once

#include <SDL2/SDL.h>

#include "game/behaviours.h"
#include "game/entities.h"
#include "std.h"

typedef struct Camera {
    Vec2 position;
    Vec2 scale;
    f32 rotation;
} Camera;

typedef struct Game {
    SDL_Window *window;
    SDL_Renderer *renderer;
    u16 windowWidth;
    u16 windowHeight;
    SDL_GameController *controller;
    Camera camera;
} Game;

int GameInit(Game *game);
int GameLoadDefaultController(Game *game);
void GameShutdown(Game *game);
