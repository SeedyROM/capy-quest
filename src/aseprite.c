#include "aseprite.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>

// ======================================================================================
// Define this to print a ton of debug info
// ======================================================================================
#define ASEPRITE_DEBUG
// ======================================================================================

#ifdef ASEPRITE_DEBUG
static inline void AsepriteDebugPrint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
#else
static inline void AsepriteDebugPrint(const char *fmt, ...)
{
    (void)fmt;
}
#endif

AsepriteFile *AsepriteLoad(Arena *arena, String *path)
{
    AsepriteDebugPrint("\n===========================================================\n");
    AsepriteDebugPrint("Loading sprite: %s\n", path->ptr);
    AsepriteDebugPrint("===========================================================\n\n");

    struct
    {
        usize offset;
        ByteArray *data;
    } spriteParser;
    spriteParser.offset = 0;
    spriteParser.data = ReadFileBytes(arena, path);

    AsepriteDebugPrint("File size: %zu\n", spriteParser.data->len);
    AsepriteFile *file = ArenaPushStruct(arena, AsepriteFile);

    // NOTE(SeedyROM): Should this be checked?
    u32 spriteSize = ByteArrayReadU32(spriteParser.data, &spriteParser.offset);
    file->size = spriteSize;
    assert(spriteSize == spriteParser.data->len);

    // Check the magic number
    u16 headerMagic = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
    assert(headerMagic == AsepriteFileMagic);

    // Check the number of frames
    u16 numFrames = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
    file->numFrames = numFrames;
    file->frames = ArenaPushArray(arena, numFrames, AsepriteFrameRaw);
    AsepriteDebugPrint("Number of frames: %u\n", numFrames);

    // Check the width and height
    u16 spriteWidth = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
    u16 spriteHeight = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
    file->width = spriteWidth;
    file->height = spriteHeight;
    AsepriteDebugPrint("Width: %u\n", spriteWidth);
    AsepriteDebugPrint("Height: %u\n", spriteHeight);

    // Skip the rest of the header
    spriteParser.offset += 116;

    u16 framesProcessed = 0;
    do
    {
        AsepriteFrameRaw *frame = &file->frames[framesProcessed];

        AsepriteDebugPrint("\n");

        // Get the size of the frame
        u32 frameSize = ByteArrayReadU32(spriteParser.data, &spriteParser.offset);
        frame->size = frameSize;
        AsepriteDebugPrint("Frame %d size: %u\n", framesProcessed, frameSize);

        // Check the frame magic number
        u16 frameMagic = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
        assert(frameMagic == AsepriteFrameMagic);

        // Old chunk count
        u16 oldNumChunks = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
        (void)oldNumChunks;

        // Frame duration in milliseconds
        u16 frameDuration = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
        frame->duration = frameDuration;
        AsepriteDebugPrint("Frame %d duration: %u\n", framesProcessed, frameDuration);

        // Ignore these next 2 bytes...
        spriteParser.offset += 2;

        // New bigger chunk count
        u32 newNumChunks = ByteArrayReadU32(spriteParser.data, &spriteParser.offset);

        // Just use the new field for now...
        u32 numChunks = newNumChunks;
        frame->numChunks = numChunks;
        frame->chunks = ArenaPushArray(arena, numChunks, AsepriteFrameChunk);

        AsepriteDebugPrint("Frame %d number of chunks: %u\n", framesProcessed, numChunks);

        u16 chunksProcessed = 0;
        do
        {
            AsepriteFrameChunk *chunk = &frame->chunks[chunksProcessed];

            AsepriteDebugPrint("\n");

            // Check the chunk size
            u32 chunkSize = ByteArrayReadU32(spriteParser.data, &spriteParser.offset);
            chunk->size = chunkSize;
            AsepriteDebugPrint("Chunk %d size: %u\n", chunksProcessed, chunkSize);

            // Check the chunk type
            u16 chunkType = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
            chunk->type = chunkType;
            AsepriteDebugPrint("Chunk %d type: %04X\n", chunksProcessed, chunkType);

            switch (chunkType)
            {
            case AsepriteChunkType_Cel:
            {
                AsepriteFrameCelChunk *celChunk = &chunk->chunk.frameCel;

                AsepriteDebugPrint("\nProcessing CelChunk\n");

                u16 layerIndex = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                celChunk->layerIndex = layerIndex;
                AsepriteDebugPrint("Layer index: %u\n", layerIndex);

                u16 positionX = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                u16 positionY = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                celChunk->positionX = positionX;
                celChunk->positionY = positionY;
                AsepriteDebugPrint("Position: (%u, %u)\n", positionX, positionY);

                u8 opacity = ByteArrayReadU8(spriteParser.data, &spriteParser.offset);
                celChunk->opacity = opacity;
                AsepriteDebugPrint("Opacity: %u\n", opacity);

                u16 celType = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                celChunk->celType = celType;
                AsepriteDebugPrint("Cel type: %u\n", celType);

                i16 zIndex = ByteArrayReadI16(spriteParser.data, &spriteParser.offset);
                celChunk->zIndex = zIndex;
                AsepriteDebugPrint("Z-index: %d\n", zIndex);

                // Skip the next 5 bytes from the FUTURE...
                spriteParser.offset += 5;

                u32 *pixels = 0;
                switch (celType)
                {
                case AsepriteCelType_RawCel:
                {
                    AsepriteDebugPrint("Unsupported CelType: RawCel\n");
                    exit(EXIT_FAILURE);
                };
                break;

                case AsepriteCelType_LinkedCel:
                {
                    AsepriteDebugPrint("Processing LinkedCel\n");

                    u16 framePosition = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                    AsepriteDebugPrint("Frame position: %u\n", framePosition);
                };
                break;

                case AsepriteCelType_CompressedImage:
                {
                    AespriteCelCompressedImage *compressedImage = &celChunk->cel.compressedImage;

                    AsepriteDebugPrint("\nProcessing CompressedImage\n");

                    u16 celWidth = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                    u16 celHeight = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                    compressedImage->width = celWidth;
                    compressedImage->height = celHeight;
                    AsepriteDebugPrint("Size: (%u, %u) = %u\n", celWidth, celHeight, celWidth * celHeight);

                    // WTF is this?
                    // It's the CelHeader size...
                    usize compressedSize = chunkSize - 26;
                    u8 *compressedData = ByteArrayReadArrayU8(spriteParser.data, &spriteParser.offset, compressedSize);

                    AsepriteDebugPrint("\nCompressed Data (Blob):\n\n");
                    // Print the compressed data
                    for (u32 i = 0; i < compressedSize; i++)
                    {
                        AsepriteDebugPrint("0x%02X ", compressedData[i]);
                        if ((i + 1) % celWidth == 0)
                        {
                            AsepriteDebugPrint("\n");
                        }
                    }
                    AsepriteDebugPrint("\n\n");

                    pixels = ArenaPushArray(arena, celWidth * celHeight, u32);

                    // Set up zlib's inflate stream
                    z_stream stream;
                    stream.zalloc = Z_NULL;
                    stream.zfree = Z_NULL;
                    stream.opaque = Z_NULL;
                    stream.avail_in = 0;
                    stream.next_in = Z_NULL;

                    // Initialize the inflate stream with the compressed data
                    int ret = inflateInit(&stream);
                    if (ret != Z_OK)
                    {
                        AsepriteDebugPrint("gzip inflateInit failed: %s\n", zError(ret));
                        exit(EXIT_FAILURE);
                    }

                    stream.avail_in = compressedSize;
                    stream.next_in = compressedData;

                    stream.avail_out = celWidth * celHeight * sizeof(u32);
                    stream.next_out = (unsigned char *)pixels;

                    // Perform decompression
                    ret = inflate(&stream, Z_FINISH);
                    if (ret != Z_STREAM_END)
                    {
                        AsepriteDebugPrint("gzip inflate failed: %s\n", zError(ret));
                        exit(EXIT_FAILURE);
                    }

                    // Clean up the inflate stream
                    inflateEnd(&stream);

                    compressedImage->pixels = pixels;

                    AsepriteDebugPrint("Decompressed Data (Pixels):\n\n");
                    // Print the decompressed data.
                    for (u32 i = 0; i < celWidth * celHeight; i++)
                    {
                        u8 r, g, b;
                        r = pixels[i] & 0xFF;
                        g = pixels[i] >> 8 & 0xFF;
                        b = pixels[i] >> 16 & 0xFF;

                        if (pixels[i] == 0)
                        {
                            AsepriteDebugPrint("  ");
                        }
                        else
                        {
                            AsepriteDebugPrint("\033[38;2;%d;%d;%dm██\033[0;00m", r, g, b);
                        }
                        if ((i + 1) % celWidth == 0)
                        {
                            AsepriteDebugPrint("\n");
                        }
                    }
                    AsepriteDebugPrint("\n");
                };
                break;

                case AespriteCelType_CompressedTileMap:
                {
                    AsepriteDebugPrint("CompressedTileMap not supported!!!\n");
                    exit(1);
                };
                break;
                }
            };
            break;

            default:
            {
                // Skip the rest of the chunk plus the chunk headers...
                AsepriteDebugPrint("Skipping chunk of type %04X\n", chunkType);
                spriteParser.offset += chunkSize - 6;
            };
            }

        } while (chunksProcessed++ < numChunks - 1);
    } while (framesProcessed++ < numFrames - 1);

    AsepriteDebugPrint("\n===========================================================\n");
    AsepriteDebugPrint("DONE loading sprite: %s\n", path->ptr);
    AsepriteDebugPrint("===========================================================\n\n");

    return file;
}

AsepriteFrame *AsepriteGetFrame(Arena *arena, AsepriteFile *file, usize frameIndex)
{
    AsepriteFrameRaw *rawFrame = &file->frames[frameIndex];

    AsepriteFrame *frame = ArenaPushStruct(arena, AsepriteFrame);
    frame->sizeX = file->width;
    frame->sizeY = file->height;
    frame->frameDuration = rawFrame->duration;
    frame->pixels = NULL;

    for (u32 chunkIndex = 0; chunkIndex < rawFrame->numChunks; chunkIndex++)
    {
        AsepriteFrameChunk *chunk = &rawFrame->chunks[chunkIndex];

        switch (chunk->type)
        {
        case AsepriteChunkType_Cel:
        {
            AsepriteFrameCelChunk *celChunk = &chunk->chunk.frameCel;

            switch (celChunk->celType)
            {
            case AsepriteCelType_CompressedImage:
            {
                AespriteCelCompressedImage *compressedImage = &celChunk->cel.compressedImage;

                frame->pixels = compressedImage->pixels;
            };
            break;

            default:
            {
                AsepriteDebugPrint("Unsupported CelType: %u\n", celChunk->celType);
                exit(EXIT_FAILURE);
            };
            break;
            }
        };
        break;

        default:
        {
            AsepriteDebugPrint("Unsupported ChunkType: %u\n", chunk->type);
        };
        break;
        }
    }

    return frame;
}
