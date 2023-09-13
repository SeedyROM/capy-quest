#pragma once

#include "fs.h"
#include "arena.h"
#include "str.h"
#include "util.h"

static const u32 AsepriteFileMagic = 0xA5E0;
static const u32 AsepriteFrameMagic = 0xF1FA;

typedef enum
{
    AsepriteChunkType_Layer = 0x2004,
    AsepriteChunkType_Cel = 0x2005,
    AsepriteChunkType_CelExtra = 0x2006,
    AsepriteChunkType_ColorProfile = 0x2007,
    AsepriteChunkType_Mask = 0x2016,
    AsepriteChunkType_Path = 0x2017,
    AsepriteChunkType_FrameTags = 0x2018,
    AsepriteChunkType_Palette = 0x2019,
    AsepriteChunkType_UserData = 0x2020,
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
} AsepriteFrameRaw;

typedef struct
{
    u16 sizeX;
    u16 sizeY;
    u16 frameDuration;
    u32 *pixels;
} AsepriteFrame;

typedef struct
{
    u32 size;
    u16 width;
    u16 height;
    u16 numFrames;
    AsepriteFrameRaw *frames;
} AsepriteFile;

void PrintSpriteToConsole(u16 celWidth, u16 celHeight, u32 *pixels);
AsepriteFile *AsepriteLoad(Arena *arena, String *path);
AsepriteFrame *AsepriteGetFrame(Arena *arena, AsepriteFile *file, usize frameIndex);
