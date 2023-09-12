#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define GL_SILENCE_DEPRECATION 1
#include <GLFW/glfw3.h>

#include <zlib.h>

#include "arena.h"
#include "fs.h"
#include "str.h"
#include "util.h"

const u32 AsepriteFileMagic = 0xA5E0;
const u32 AsepriteFrameMagic = 0xF1FA;

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

int main(void)
{
    Arena *arena = ArenaAlloc(512 * Megabyte);

    Arena *spriteDataArena = ArenaAlloc(16 * Megabyte);
    tempMemoryBlock(spriteDataArena)
    {
        // NOTE(SeedyROM): this is just an excuse to test the StringBuilder
        StringBuilder *sb = StringBuilderAlloc(spriteDataArena);
        StringBuilderAppend(sb, STR("../assets/sprites/"));
        StringBuilderAppend(sb, STR("capy1.aseprite"));
        String path = sb->string;

        struct
        {
            usize offset;
            ByteArray *data;
        } spriteParser;
        spriteParser.offset = 0;
        spriteParser.data = ReadFileBytes(spriteDataArena, &path);
        printf("File size: %zu\n", spriteParser.data->len);

        // NOTE(SeedyROM): Should this be checked?
        u32 spriteSize = ByteArrayReadU32(spriteParser.data, &spriteParser.offset);
        assert(spriteSize == spriteParser.data->len);

        // Check the magic number
        u16 headerMagic = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
        assert(headerMagic == AsepriteFileMagic);

        // Check the number of frames
        u16 numFrames = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
        printf("Number of frames: %u\n", numFrames);

        // Check the width and height
        u16 spriteWidth = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
        u16 spriteHeight = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
        printf("Width: %u\n", spriteWidth);
        printf("Height: %u\n", spriteHeight);

        // Skip the rest of the header
        spriteParser.offset += 116;

        u16 framesProcessed = 0;
        do
        {
            printf("\n");

            // Get the size of the frame
            u32 frameSize = ByteArrayReadU32(spriteParser.data, &spriteParser.offset);
            printf("Frame %d size: %u\n", framesProcessed, frameSize);

            // Check the frame magic number
            u16 frameMagic = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
            assert(frameMagic == AsepriteFrameMagic);

            // Old chunk count
            u16 oldNumChunks = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);

            // Frame duration in milliseconds
            u16 frameDuration = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
            printf("Frame %d duration: %u\n", framesProcessed, frameDuration);

            // Ignore these next 2 bytes...
            spriteParser.offset += 2;

            // New bigger chunk count
            u32 newNumChunks = ByteArrayReadU32(spriteParser.data, &spriteParser.offset);

            // Just use the new field for now...
            u32 numChunks = newNumChunks;

            printf("Frame %d number of chunks: %u\n", framesProcessed, numChunks);

            u16 chunksProcessed = 0;
            do
            {
                printf("\n");

                // Check the chunk size
                u32 chunkSize = ByteArrayReadU32(spriteParser.data, &spriteParser.offset);
                printf("Chunk %d size: %u\n", chunksProcessed, chunkSize);

                // Check the chunk type
                u16 chunkType = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                printf("Chunk %d type: %04X\n", chunksProcessed, chunkType);

                switch (chunkType)
                {
                case AsepriteChunkType_CelChunk:
                {
                    printf("\nProcessing CelChunk\n");

                    u16 layerIndex = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                    printf("Layer index: %u\n", layerIndex);

                    u16 x = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                    u16 y = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                    printf("Position: (%u, %u)\n", x, y);

                    u8 opacity = ByteArrayReadU8(spriteParser.data, &spriteParser.offset);
                    printf("Opacity: %u\n", opacity);

                    u16 celType = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                    printf("Cel type: %u\n", celType);

                    i16 zIndex = ByteArrayReadI16(spriteParser.data, &spriteParser.offset);

                    // Skip the next 5 bytes from the FUTURE...
                    spriteParser.offset += 5;

                    u8 *pixels = NULL;
                    switch (celType)
                    {
                    case AsepriteCelType_RawCel:
                    {
                        printf("Processing RawCel\n");

                        u16 celWidth = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                        u16 celHeight = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                        printf("Size: (%u, %u)\n", celWidth, celHeight);

                        u8 *pixels = ByteArrayReadArrayU8(spriteParser.data, &spriteParser.offset, celWidth * celHeight);
                    };
                    break;

                    case AsepriteCelType_LinkedCel:
                    {
                        printf("Processing LinkedCel\n");

                        u16 framePosition = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                        printf("Frame position: %u\n", framePosition);
                    };
                    break;

                    case AsepriteCelType_CompressedImage:
                    {
                        printf("Processing CompressedImage\n");

                        u16 celWidth = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                        u16 celHeight = ByteArrayReadU16(spriteParser.data, &spriteParser.offset);
                        printf("Size: (%u, %u)\n", celWidth, celHeight);

                        u8 *compressedData = ByteArrayReadArrayU8(spriteParser.data, &spriteParser.offset, celWidth * celHeight);

                        printf("\nCompressed Data:\n");
                        // Print the compressed data
                        for (u32 i = 0; i < celWidth * celHeight; i++)
                        {
                            printf("%02X ", compressedData[i]);
                            if ((i + 1) % celWidth == 0)
                            {
                                printf("\n");
                            }
                        }
                        printf("\n");

                        u32 *decompressedData = ArenaPushArray(spriteDataArena, spriteWidth * spriteHeight, u32);

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
                            fprintf(stderr, "inflateInit failed: %s\n", zError(ret));
                            exit(EXIT_FAILURE);
                        }

                        stream.avail_in = celWidth * celHeight;
                        stream.next_in = compressedData;

                        stream.avail_out = spriteWidth * spriteHeight * 8;
                        stream.next_out = decompressedData;

                        // Perform decompression
                        ret = inflate(&stream, Z_FINISH);
                        if (ret != Z_STREAM_END)
                        {
                            fprintf(stderr, "inflate failed: %s\n", zError(ret));
                            exit(EXIT_FAILURE);
                        }

                        // Clean up the inflate stream
                        inflateEnd(&stream);

                        printf("\nDecompressed Data:\n");
                        // Print the decompressed data
                        for (u32 i = 0; i < spriteWidth * spriteHeight; i++)
                        {
                            printf("%8X ", decompressedData[i]);
                            if ((i + 1) % spriteWidth == 0)
                            {
                                printf("\n");
                            }
                        }
                        printf("\n");
                    };
                    break;

                    case AespriteCelType_CompressedTileMap:
                    {
                        printf("CompressedTimeMap not supported!!!\n");
                        exit(1);
                    };
                    break;
                    }
                };
                break;

                default:
                {
                    printf("Skipping chunk of type %04X\n", chunkType);
                };
                }

                // Skip the rest of the chunk plus the chunk headers...
                spriteParser.offset += chunkSize - 6;
            } while (chunksProcessed++ < numChunks - 1);
        } while (framesProcessed++ < numFrames - 1);
    };
    ArenaFree(spriteDataArena);

    GLFWwindow *window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Close if q or escape is pressed
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    ArenaFree(arena);
    return 0;
}
