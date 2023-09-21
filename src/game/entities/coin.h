#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "std/gfx.h"
#include "std/util.h"

typedef struct Coin
{
    TextureAtlas *atlas;
    bool collected;
    Sprite sprite;
    usize time;
    usize collectedTime;
    u16 frameDuration;
    u16 currentFrame;
    bool delete;
} Coin;

void CoinInit(Coin *coin, TextureAtlas *atlas);
void CoinCollect(Coin *coin);
void CoinDraw(Coin *coin, SDL_Renderer *renderer);

void CoinUpdate(Coin *coin);
