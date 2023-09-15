#include <stdio.h>
#include <stdbool.h>
#include <glob.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rectpack.h>

#include "arena.h"
#include "aseprite.h"
#include "fs.h"
#include "str.h"
#include "util.h"

typedef struct SpriteAssetPath
{
    String name;
    String path;
} SpriteAssetPath;

int main(void)
{
    Arena *arena = ArenaAlloc(512 * Megabyte);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    // SDL_Surface *capySurface;

    SDL_Texture *atlasTexture;
    u16 atlasWidth = 0;
    u16 atlasHeight = 0;

    tempMemoryBlock(arena)
    {
        // Path to glob for sprites
        String spriteAssetsGlob = STR("../assets/sprites/*.aseprite");

        // Sprite assets memory
        ARRAY(SpriteAssetPath, spriteAssetPaths);

        {
            // Glob all the sprite assets
            glob_t globResult;
            glob(spriteAssetsGlob.ptr, GLOB_TILDE, NULL, &globResult);

            // Allocate space for the paths
            spriteAssetPaths.len = globResult.gl_pathc;
            spriteAssetPaths.ptr = ArenaPushArrayZero(arena, spriteAssetPaths.len, SpriteAssetPath);

            printf("Found %zu sprite assets\n", spriteAssetPaths.len);

            // Get the paths;
            for (usize i = 0; i < spriteAssetPaths.len; i++)
            {
                // Copy each path from the glob
                String *pathString = StringCopyCString(arena, globResult.gl_pathv[i]);
                spriteAssetPaths.ptr[i].path = *pathString;

                // Get the name of the sprite asset without the extension and path
                String *nameString = StringCopy(arena, pathString);
                u64 lastSlash = StringFindLastOccurrence(nameString, '/') + 1;
                u64 lastDot = StringFindLastOccurrence(nameString, '.') - 1;
                StringSlice(nameString, lastSlash, lastDot);

                // Print the sprite asset name
                printf("Sprite asset: %s\n", nameString->ptr);

                // Put the name in the sprite asset path
                spriteAssetPaths.ptr[i].name = *nameString;
            }
        }

        // Load the sprite assets
        ARRAY_ALLOC_RESERVED(arena, AsepriteFile, sprites, spriteAssetPaths.len);

        // Allocate space for the sprite frames
        ARRAY_ALLOC(arena, AsepriteAnimationFrame, spriteFrames, 128);

        for (usize i = 0; i < sprites.len; i++)
        {
            // Read the sprite asset file
            String *spriteAssetPath = &spriteAssetPaths.ptr[i].path;
            String *spriteAssetName = &spriteAssetPaths.ptr[i].name;

            printf("Loading sprite asset: %s = %s\n", spriteAssetName->ptr, spriteAssetPath->ptr);
            AsepriteFile *sprite = &sprites.ptr[i];
            sprite = AsepriteLoad(arena, spriteAssetPath);

            // Get all the the frames
            for (usize j = 0; j < sprite->numFrames; j++)
            {
                AsepriteAnimationFrame spriteFrameProcessed;
                AsepriteGetAnimationFrame(sprite, j, &spriteFrameProcessed);
                ARRAY_PUSH(arena, spriteFrames, AsepriteAnimationFrame, spriteFrameProcessed);
            }
        }

        ARRAY_ALLOC_RESERVED(arena, stbrp_rect, spriteRects, spriteFrames.len);
        for (usize i = 0; i < spriteFrames.len; i++)
        {
            stbrp_rect *rect = &spriteRects.ptr[i];
            rect->id = i;
            rect->w = spriteFrames.ptr[i].sizeX;
            rect->h = spriteFrames.ptr[i].sizeY;
        }

        // Pack the sprite frames using stb_rectpack
        stbrp_context context;
        stbrp_node *nodes = ArenaPushArray(arena, spriteFrames.len, stbrp_node);
        stbrp_init_target(&context, spriteRects.len * 8, INT32_MAX, nodes, spriteRects.len);
        int result = stbrp_pack_rects(&context, spriteRects.ptr, spriteRects.len);
        if (result == 0)
        {
            printf("Failed to pack sprite frames\n");
            return 1;
        }

        // Get the size of the atlas
        for (usize i = 0; i < spriteFrames.len; i++)
        {
            stbrp_rect *rect = &spriteRects.ptr[i];
            if (rect->x + rect->w > atlasWidth)
            {
                atlasWidth = rect->x + rect->w;
            }

            if (rect->y + rect->h > atlasHeight)
            {
                atlasHeight = rect->y + rect->h;
            }
        }

        // Allocate space for the atlas
        u32 *atlasPixels = ArenaPushArrayZero(arena, atlasWidth * atlasHeight, u32);

        // Copy the sprite frames into the atlas
        for (usize i = 0; i < spriteFrames.len; i++)
        {
            stbrp_rect *rect = &spriteRects.ptr[i];
            AsepriteAnimationFrame *spriteFrame = &spriteFrames.ptr[rect->id];

            for (u16 y = 0; y < spriteFrame->sizeY; y++)
            {
                for (u16 x = 0; x < spriteFrame->sizeX; x++)
                {
                    u32 *atlasPixel = &atlasPixels[(rect->y + y) * atlasWidth + (rect->x + x)];
                    u32 *spritePixel = &spriteFrame->pixels[y * spriteFrame->sizeX + x];

                    *atlasPixel = *spritePixel;
                }
            }
        }

        // Create a texture from the atlas
        atlasTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, atlasWidth, atlasHeight);

        // Copy the atlas pixels into the texture
        SDL_UpdateTexture(atlasTexture, NULL, atlasPixels, atlasWidth * sizeof(u32));

        // Setup 0x00000000 as the transparent color
        SDL_SetTextureBlendMode(atlasTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(atlasTexture, 0x00000000);
    }

    SDL_Event event;
    bool running = true;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 128, 200, 255);
        SDL_RenderClear(renderer);

        // Draw the atlas
        SDL_Rect atlasRect = {0, 0, atlasWidth * 4, atlasHeight * 4};
        SDL_RenderCopy(renderer, atlasTexture, NULL, &atlasRect);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    ArenaFree(arena);

    return 0;
}
