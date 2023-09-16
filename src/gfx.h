#pragma once

#include <SDL2/SDL.h>

#include "util.h"
#include "str.h"

typedef SDL_Rect TextureAtlasFrame;

typedef struct TextureAtlasIndex
{
    u16 numFrames;
    u32 frameIndex;
    String *name;
} TextureAtlasIndex;

DEFINE_ARRAY(TextureAtlasFrame, TextureAtlasFrames);
DEFINE_ARRAY(TextureAtlasIndex, TextureAtlasIndices);

typedef struct TextureAtlas
{
    Arena *arena;
    TextureAtlasIndices indices;
    TextureAtlasFrames frames;
    SDL_Texture *texture;
    u16 width;
    u16 height;
} TextureAtlas;

TextureAtlas *TextureAtlasInit(Arena *arena);
int TextureAtlasLoad(SDL_Renderer *renderer, TextureAtlas *atlas, String *path);
i64 TextureAtlasIndicesGetIndex(TextureAtlas *atlas, String *name);
TextureAtlasFrames TextureAtlasIndicesGetFrames(TextureAtlas *atlas, String *name);
