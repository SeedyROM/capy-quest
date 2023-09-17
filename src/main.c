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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
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
    int windowScaleFactor = 4;

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
    Vec2 capyVelocity = {0, 0};

    // Gravity
    f32 gravity = 0.05f;

    // Dumb timer
    u64 time = 0;

    // Setup the joystick
    SDL_Joystick *joystick = NULL;

    // Print how many joysticks are connected
    printf("Number of joysticks connected: %d\n", SDL_NumJoysticks());

    if (SDL_NumJoysticks() > 0)
    {
        joystick = SDL_JoystickOpen(0);
        if (joystick == NULL)
        {
            fprintf(stderr, "SDL_JoystickOpen Error: %s\n", SDL_GetError());
        }
    }

    // Print the joystick name
    printf("Joystick name: %s\n", SDL_JoystickName(joystick));

    // Loop de loop
    SDL_Event event;
    bool running = true;
    while (running)
    {
        for (unsigned int i = 0; i < SDL_JoystickNumAxes(joystick); ++i)
        {
            int axis = SDL_JoystickGetAxis(joystick, i);
            printf("a%d: %d\n", i, axis);
        }

        bool isUpPressed = false;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_UP)
                {
                    isUpPressed = true;
                }
            }
        }

        // Gravity
        capyVelocity.y += gravity;

        const f32 maxSpeed = 2.0;
        // Move with arrow keys
        const Uint8 *state = SDL_GetKeyboardState(NULL);

        // Move with joystick
        if (joystick != NULL)
        {
            isUpPressed = false;

            // Get the joystick state
            SDL_JoystickUpdate();
            Sint16 xAxis = SDL_JoystickGetAxis(joystick, 0);
            Sint16 yAxis = SDL_JoystickGetAxis(joystick, 1);

            // printf("xAxis: %d, yAxis: %d\n", xAxis, yAxis);
            // printf("Button 0: %d\n", SDL_JoystickGetButton(joystick, 0));

            // Deadzone
            if (xAxis > -1000 && xAxis < 1000)
            {
                xAxis = 0;
            }
            if (yAxis > -1000 && yAxis < 1000)
            {
                yAxis = 0;
            }

            // Move
            if (xAxis < 0)
            {
                if (capyVelocity.x > -maxSpeed)
                    capyVelocity.x -= 0.05f;
                else if (capyVelocity.x < -maxSpeed)
                    capyVelocity.x = -maxSpeed;
                capySprite->flipX = true;
            }
            if (xAxis > 0)
            {
                if (capyVelocity.x < maxSpeed)
                    capyVelocity.x += 0.05f;
                else if (capyVelocity.x > maxSpeed)
                    capyVelocity.x = maxSpeed;
                capySprite->flipX = false;
            }

            isUpPressed = SDL_JoystickGetButton(joystick, 0);
            // Jump
            if (yAxis < 0)
            {
                if (isUpPressed)
                {
                    capyVelocity.y = -1.2;
                    isUpPressed = false;
                }
            }
        }

        if (state[SDL_SCANCODE_LEFT])
        {
            if (capyVelocity.x > -maxSpeed)
                capyVelocity.x -= 0.05f;
            else if (capyVelocity.x < -maxSpeed)
                capyVelocity.x = -maxSpeed;
            capySprite->flipX = true;
        }
        if (state[SDL_SCANCODE_RIGHT])
        {
            if (capyVelocity.x < maxSpeed)
                capyVelocity.x += 0.05f;
            else if (capyVelocity.x > maxSpeed)
                capyVelocity.x = maxSpeed;
            capySprite->flipX = false;
        }

        // If left and right are not pressed, slow down
        if (!state[SDL_SCANCODE_LEFT] && !state[SDL_SCANCODE_RIGHT])
        {
            if (capyVelocity.x > 0)
            {
                capyVelocity.x -= 0.05f;
                if (capyVelocity.x < 0)
                    capyVelocity.x = 0;
            }
            else if (capyVelocity.x < 0)
            {
                capyVelocity.x += 0.05f;
                if (capyVelocity.x > 0)
                    capyVelocity.x = 0;
            }
        }

        // Jump
        if (state[SDL_SCANCODE_UP])
        {
            if (isUpPressed)
            {
                capyVelocity.y = -1.2;
                isUpPressed = false;
            }
        }

        // Apply friction
        // capyVelocity.x *= 0.9f;

        // Update position
        capySprite->pos.x += capyVelocity.x;
        capySprite->pos.y += capyVelocity.y;

        // Clamp position to screen
        if (capySprite->pos.x < 0)
        {
            capySprite->pos.x = 0;
        }
        if (capySprite->pos.x > windowWidth - capySprite->frames.ptr[capySprite->currentFrame].w)
        {
            capySprite->pos.x = windowWidth - capySprite->frames.ptr[capySprite->currentFrame].w;
        }

        // Clamp position to screen y
        if (capySprite->pos.y < 0)
        {
            capySprite->pos.y = 0;
        }
        if (capySprite->pos.y > windowHeight - capySprite->frames.ptr[capySprite->currentFrame].h)
        {
            capySprite->pos.y = windowHeight - capySprite->frames.ptr[capySprite->currentFrame].h;
        }

        if (time % 20 == 0)
        {
            SpriteNextFrame(capySprite);
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
