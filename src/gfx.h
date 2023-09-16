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

i64 TextureAtlasIndicesGetIndex(TextureAtlasIndices *indices, String *name);
TextureAtlasFrames TextureAtlasIndicesGetFrames(TextureAtlasIndices *indices, TextureAtlasFrames *frames, String *name);
