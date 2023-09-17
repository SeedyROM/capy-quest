#pragma once

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "util.h"
#include "str.h"

// TODO(SeedyROM): Frames need durations... fuck
typedef SDL_Rect TextureAtlasFrame;

typedef struct TextureAtlasIndex
{
    u16 numFrames;
    u32 frameIndex;
    String *name;
} TextureAtlasIndex;

ARRAY_DEFINE(TextureAtlasFrame, TextureAtlasFrames);
ARRAY_DEFINE(TextureAtlasIndex, TextureAtlasIndices);

typedef struct TextureAtlas
{
    Arena *arena;
    TextureAtlasIndices indices;
    TextureAtlasFrames frames;
    SDL_Texture *texture;
    u16 width;
    u16 height;
} TextureAtlas;

TextureAtlas *TextureAtlasCreate(Arena *arena);
int TextureAtlasLoadSprites(SDL_Renderer *renderer, TextureAtlas *atlas, String *path);
i64 TextureAtlasIndicesGetIndex(TextureAtlas *atlas, String *name);
TextureAtlasFrames TextureAtlasIndicesGetFrames(TextureAtlas *atlas, String *name);

typedef struct Sprite
{
    TextureAtlas *atlas;
    TextureAtlasFrames frames;
    u16 currentFrame;
    Vec2 pos;
    Vec2 scale;
    f32 rotation;
    bool flipX;
    bool flipY;
} Sprite;

Sprite *SpriteFromAtlas(Arena *arena, TextureAtlas *atlas, String *name);
void SpriteDraw(Sprite *sprite, SDL_Renderer *renderer);
void SpriteNextFrame(Sprite *sprite);
void SpritePreviousFrame(Sprite *sprite);
