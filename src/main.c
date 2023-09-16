#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "arena.h"
#include "gfx.h"
#include "str.h"
#include "util.h"

int main(void)
{
    Arena *arena = ArenaAlloc(128 * Megabyte);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Pixel Art Hint
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Capy Quest", 100, 100, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
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

    // Window scale factor
    int windowScaleFactor = 8;

    // Get the window size
    int windowRealWidth = 0;
    int windowRealHeight = 0;
    SDL_GetWindowSize(window, &windowRealWidth, &windowRealHeight);

    // Get the scaled window size
    int windowWidth = windowRealWidth / windowScaleFactor;
    int windowHeight = windowRealHeight / windowScaleFactor;

    // Set the logical size of the renderer
    SDL_RenderSetLogicalSize(renderer, windowWidth, windowHeight);

    // Load the texture atlas
    TextureAtlas *textureAtlas = TextureAtlasInit(arena);
    TextureAtlasLoad(renderer, textureAtlas, &STR("../assets/sprites/*.aseprite"));

    // Get the capy sprite
    Sprite *capySprite = SpriteFromAtlas(arena, textureAtlas, &STR("capy"));

    // Dumb timer
    u64 time = 0;

    // Loop de loop
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

        // Move with arrow keys
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_LEFT])
        {
            capySprite->pos.x -= 1;
            capySprite->flipX = true;
        }
        if (state[SDL_SCANCODE_RIGHT])
        {
            capySprite->pos.x += 1;
            capySprite->flipX = false;
        }
        if (state[SDL_SCANCODE_UP])
        {
            capySprite->pos.y -= 1;
        }
        if (state[SDL_SCANCODE_DOWN])
        {
            capySprite->pos.y += 1;
        }

        if (time % 20 == 0)
        {
            SpriteAdvanceFrame(capySprite);
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 128, 200, 255);
        SDL_RenderClear(renderer);

        // Draw a rect from the atlas texture
        SpriteDraw(capySprite, renderer);

        // Update the screen
        SDL_RenderPresent(renderer);

        // 60 FPS
        SDL_Delay(16);

        time++;
    }

    // Shutdown SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // Clean up memory
    ArenaFree(arena);

    return 0;
}
