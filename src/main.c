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

int main(void)
{
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

    Arena *arena = ArenaAlloc(512 * Megabyte);
    tempMemoryBlock(arena)
    {
        // Path to glob for sprites
        String spriteAssetsGlob = STR("../assets/sprites/*.aseprite");

        // Glob all the sprite assets
        glob_t globResult;
        glob(spriteAssetsGlob.ptr, GLOB_TILDE, NULL, &globResult);

        // Allocate space for the paths
        struct
        {
            u32 len;
            String *ptr;
        } spriteAssets;
        spriteAssets.len = globResult.gl_pathc;
        spriteAssets.ptr = ArenaPushArrayZero(arena, spriteAssets.len, String);

        printf("Found %d sprite assets\n", spriteAssets.len);

        // Get the paths;
        for (usize i = 0; i < spriteAssets.len; i++)
        {
            // Copy each path from the glob
            String *pathString = StringCopyCString(arena, globResult.gl_pathv[i]);
            spriteAssets.ptr[i] = *pathString;
        }

        // Free the glob result
        globfree(&globResult);

        for (usize i = 0; i < spriteAssets.len; i++)
        {
            printf("%s\n", spriteAssets.ptr[i].ptr);
        }

        // Load the sprite
        // AsepriteFile *file = AsepriteLoad(arena, &path);

        // {
        //     // Get all the animation frames
        //     struct
        //     {
        //         AsepriteAnimationFrame *ptr;
        //         u16 cap;
        //     } animationFrames;

        //     animationFrames.ptr = ArenaPushArrayZero(arena, file->numFrames, AsepriteAnimationFrame);
        //     for (u16 frameIndex = 0; frameIndex < file->numFrames; frameIndex++)
        //     {
        //         AsepriteGetAnimationFrame(file, frameIndex, &animationFrames.ptr[frameIndex]);
        //     }

        //     stbrp_rect *rects = ArenaPushArrayZero(arena, file->numFrames, stbrp_rect);
        //     for (u16 frameIndex = 0; frameIndex < file->numFrames; frameIndex++)
        //     {
        //         AsepriteAnimationFrame *frame = &animationFrames.ptr[frameIndex];
        //         rects[frameIndex].id = frameIndex;
        //         rects[frameIndex].w = frame->sizeX;
        //         rects[frameIndex].h = frame->sizeY;
        //     }
        // }

        {
            // Build a texture atlas with stb_rectpack
            // struct
            // {
            //     u16 width;
            //     u16 height;
            //     u16 numFrames;
            //     u16 numPixels;
            //     u32 *pixels;
            // } atlas;

            // atlas.width = 0;
            // atlas.height = 0;
            // atlas.numFrames = file->numFrames;
            // atlas.numPixels = 0;

            // for (u16 frameIndex = 0; frameIndex < file->numFrames; frameIndex++)
            // {
            //     AsepriteAnimationFrame *frame = &animationFrames.ptr[frameIndex];
            //     atlas.width += frame->sizeX;
            //     atlas.height = MAX(atlas.height, frame->sizeY);
            //     atlas.numPixels += frame->sizeX * frame->sizeY;
            // }

            // atlas.pixels = ArenaPushArrayZero(arena, atlas.numPixels, u32);

            // // Copy the pixels into the atlas
            // u16 x = 0;
            // for (u16 frameIndex = 0; frameIndex < file->numFrames; frameIndex++)
            // {
            //     AsepriteAnimationFrame *frame = &animationFrames.ptr[frameIndex];
            //     for (u16 y = 0; y < frame->sizeY; y++)
            //     {
            //         memcpy(atlas.pixels + (y * atlas.width) + x, frame->pixels + (y * frame->sizeX), frame->sizeX * sizeof(u32));
            //     }
            //     x += frame->sizeX;
            // }

            // // Create the capy surface
            // capySurface = SDL_CreateRGBSurfaceFrom(atlas.pixels, atlas.width, atlas.height, 32, atlas.width * sizeof(u32), 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        }
    }

    // Draw the capy sprite
    // SDL_Texture *capyTexture = SDL_CreateTextureFromSurface(renderer, capySurface);
    // if (capyTexture == NULL)
    // {
    //     fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
    //     return 1;
    // }

    // u16 posX = 0;

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

        // SDL_Rect capyRect = {posX++, 0, capySurface->w * 4, capySurface->h * 4};
        // SDL_RenderCopy(renderer, capyTexture, NULL, &capyRect);

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
