#pragma once

#include "fs.h"
#include "arena.h"
#include "str.h"
#include "util.h"

static const u32 AsepriteFileMagic = 0xA5E0;
static const u32 AsepriteFrameMagic = 0xF1FA;

typedef enum
{
    AsepriteChunkType_LayerChunk = 0x2004,
    AsepriteChunkType_CelChunk = 0x2005,
    AsepriteChunkType_CelExtraChunk = 0x2006,
    AsepriteChunkType_ColorProfileChunk = 0x2007,
    AsepriteChunkType_MaskChunk = 0x2016,
    AsepriteChunkType_PathChunk = 0x2017,
    AsepriteChunkType_FrameTagsChunk = 0x2018,
    AsepriteChunkType_PaletteChunk = 0x2019,
    AsepriteChunkType_UserDataChunk = 0x2020,
} AsepriteChunkType;

typedef enum
{
    AsepriteCelType_RawCel = 0,
    AsepriteCelType_LinkedCel = 1,
    AsepriteCelType_CompressedImage = 2,
    AespriteCelType_CompressedTileMap = 3,
} AsepriteCelType;

typedef struct
{
    u16 width;
    u16 height;
    u32 *pixels;
} AespriteCelCompressedImage;

typedef struct
{
    u16 layerIndex;
    u16 positionX;
    u16 positionY;
    u8 opacity;
    u16 celType;
    i16 zIndex;

    union
    {
        AespriteCelCompressedImage compressedImage;
    } cel;
} AsepriteFrameCelChunk;

typedef struct
{
    u32 size;
    AsepriteChunkType type;

    union
    {
        AsepriteFrameCelChunk frameCel;
    } chunk;
} AsepriteFrameChunk;

typedef struct
{
    u32 size;
    u16 duration;
    u16 numChunks;
    AsepriteFrameChunk *chunks;
} AsepriteFrame;

typedef struct
{
    u32 size;
    u16 width;
    u16 height;
    u16 numFrames;
    AsepriteFrame *frames;
} AsepriteFile;

AsepriteFile *AsepriteLoad(Arena *arena, String *path);
