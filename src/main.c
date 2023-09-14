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
        ARRAY(AsepriteFile, sprites);
        sprites.len = spriteAssetPaths.len;
        sprites.ptr = ArenaPushArrayZero(arena, sprites.len, AsepriteFile);

        for (usize i = 0; i < sprites.len; i++)
        {
            // Read the sprite asset file
            String *spriteAssetPath = &spriteAssetPaths.ptr[i].path;
            String *spriteAssetName = &spriteAssetPaths.ptr[i].name;

            printf("Loading sprite asset: %s\n", spriteAssetPath->ptr);
            AsepriteFile *sprite = &sprites.ptr[i];
            sprite = AsepriteLoad(arena, spriteAssetPath);
        }
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
