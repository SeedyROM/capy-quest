#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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

    SDL_Surface *capySurface;

    Arena *arena = ArenaAlloc(512 * Megabyte);
    tempMemoryBlock(arena)
    {
        // NOTE(SeedyROM): this is just an excuse to test the StringBuilder
        StringBuilder *sb = StringBuilderAlloc(arena);
        StringBuilderAppend(sb, STR("../assets/sprites/"));
        StringBuilderAppend(sb, STR("capy1.aseprite"));
        String path = sb->string;

        // Load the aesprite file and print some info
        AsepriteFile *file = AsepriteLoad(arena, &path);
        for (u16 frameIndex = 0; frameIndex < file->numFrames; frameIndex++)
        {
            AsepriteAnimationFrame frame;
            AsepriteGetAnimationFrame(file, frameIndex, &frame);
            printf("Frame %d: %dx%d\n", frameIndex, frame.sizeX, frame.sizeY);

            // Take the raw pixels and create a surface
            capySurface = SDL_CreateRGBSurfaceFrom(frame.pixels, frame.sizeX, frame.sizeY, 32, frame.sizeX * 4, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            if (capySurface == NULL)
            {
                fprintf(stderr, "SDL_CreateRGBSurfaceFrom Error: %s\n", SDL_GetError());
                return 1;
            }
        }
    }

    // Draw the capy sprite
    SDL_Texture *capyTexture = SDL_CreateTextureFromSurface(renderer, capySurface);
    if (capyTexture == NULL)
    {
        fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        return 1;
    }

    u16 posX = 0;

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
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect capyRect = {posX++, 0, capySurface->w * 4, capySurface->h * 4};
        SDL_RenderCopy(renderer, capyTexture, NULL, &capyRect);

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
