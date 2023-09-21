#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "game/entities.h"
#include "game/behaviours.h"
#include "std/entity.h"

static int windowWidth = 0;
static int windowHeight = 0;
static f32 gravity = 0.05f;

typedef struct Camera
{
    Vec2 position;
    Vec2 scale;
    float rotation;
} Camera;

bool IsCollision(Sprite *a, Sprite *b)
{
    SDL_Rect aRect = a->frames.ptr[a->currentFrame];
    SDL_Rect bRect = b->frames.ptr[b->currentFrame];

    // Use the center of the sprite for collision detection
    aRect.x = a->pos.x + (aRect.w / 2);
    aRect.y = a->pos.y + (aRect.h / 2);
    bRect.x = b->pos.x + (bRect.w / 2);
    bRect.y = b->pos.y + (bRect.h / 2);

    if (aRect.x + aRect.w < bRect.x || bRect.x + bRect.w < aRect.x)
    {
        return false;
    }

    if (aRect.y + aRect.h < bRect.y || bRect.y + bRect.h < aRect.y)
    {
        return false;
    }

    return true;
}

int main(void)
{
    Arena *arena = ArenaAlloc(128 * Megabyte);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
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
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
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
    windowWidth = windowRealWidth / windowScaleFactor;
    windowHeight = windowRealHeight / windowScaleFactor;

    // Set the logical size of the renderer
    SDL_RenderSetLogicalSize(renderer, windowWidth, windowHeight);

    // Setup the joystick
    SDL_GameController *controller = NULL;
    // Load the joystick mapping
    if (SDL_GameControllerAddMappingsFromFile("../assets/gamecontrollerdb.txt") == -1)
    {
        fprintf(stderr, "SDL_GameControllerAddMappingsFromFile Error: %s\n", SDL_GetError());
    }
    // Print how many joysticks are connected
    printf("Number of joysticks connected: %d\n", SDL_NumJoysticks());
    if (SDL_NumJoysticks() > 0)
    {
        controller = SDL_GameControllerOpen(0);
        if (controller == NULL)
        {
            fprintf(stderr, "SDL_JoystickOpen Error: %s\n", SDL_GetError());
        }
    }

    if (controller != NULL)
    {
        // Print the joystick name
        printf("Controller name: %s\n", SDL_GameControllerName(controller));
    }

    // Load the texture atlas
    TextureAtlas *textureAtlas = TextureAtlasCreate(arena);
    TextureAtlasLoadSprites(renderer, textureAtlas, &STR("../assets/sprites/*.aseprite"));

    // Create the camera and set the position
    Camera camera;
    camera.position = (Vec2){0, 0};
    camera.scale = (Vec2){1, 1};
    camera.rotation = 0;

    // Setup the player
    Player player;
    PlayerInit(&player, textureAtlas, &STR("capy_idle"));

    // Setup the player control
    Controllable playerControl;
    ControllableInit(&playerControl, &player.sprite.pos, &player.velocity, &player.grounded, &PlayerControl);

    // Coin entity list
    EntityList coinList;
    EntityListInit(arena, &coinList, sizeof(Coin), 32);

    // Get the wall sprite
    Sprite wallSprite;
    SpriteFromAtlas(&wallSprite, textureAtlas, &STR("rock"));

    int map[32][16] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1},
        {1, 3, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 1},
        {1, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 0, 0, 1, 1, 1, 1, 0, 0, 2, 0, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 1, 0, 2, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };

    Wall walls[128];
    int wallCount = 0;

    // Add the objects to the map
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 16; x++)
        {
            int tile = map[y][x];
            if (tile == 1)
            {
                Wall wall;
                WallInit(&wall, (Vec2){x * 16, y * 16});
                walls[wallCount] = wall;
                wallCount++;
            }

            if (tile == 2)
            {
                // Create a coin sprite, don't alloc.
                Coin coin;
                CoinInit(&coin, textureAtlas);

                // Put a coin randomly on the screen
                coin.sprite.pos.x = x * 16 + 4;
                coin.sprite.pos.y = y * 16 + 4;

                EntityListAdd(&coinList, &coin);
            }

            if (tile == 3)
            {
                player.sprite.pos.x = x * 16;
                player.sprite.pos.y = y * 16;
            }
        }
    }

    SDL_Rect lastPlayerRect = {0, 0, 0, 0};

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

            // Add the controller if it's plugged in
            if (event.type == SDL_CONTROLLERDEVICEADDED)
            {
                controller = SDL_GameControllerOpen(0);
                if (controller == NULL)
                {
                    fprintf(stderr, "SDL_JoystickOpen Error: %s\n", SDL_GetError());
                }
            }

            // Remove the controller if it's unplugged
            if (event.type == SDL_CONTROLLERDEVICEREMOVED)
            {
                SDL_GameControllerClose(controller);
                controller = NULL;
            }
        }

        if (time % 20 == 0)
        {
            SpriteNextFrame(&player.sprite);
        }

        // Update the player
        ControllableUpdate(&playerControl, controller);
        PlayerUpdate(&player, gravity);

        SDL_Rect playerRect = player.sprite.frames.ptr[player.sprite.currentFrame];
        playerRect.x = player.sprite.pos.x;
        playerRect.y = player.sprite.pos.y;

        // Resolve collisions with walls
        for (int i = 0; i < wallCount; i++)
        {
            Wall wall = walls[i];

            SDL_Rect wallRect = wallSprite.frames.ptr[wallSprite.currentFrame];
            wallRect.x = wall.position.x;
            wallRect.y = wall.position.y;

            SDL_Rect overlap = {0, 0, 0, 0};

            if (!SDL_IntersectRect(&playerRect, &wallRect, &overlap))
            {
                player.grounded = false;
                continue;
            }

            // If the sprite was previous above the platform
            if (lastPlayerRect.y + lastPlayerRect.h <= wallRect.y)
            {
                // If the sprite is now inside the platform
                if (playerRect.y + playerRect.h > wallRect.y)
                {
                    // Move the sprite up
                    player.sprite.pos.y = wallRect.y - playerRect.h;
                    player.velocity.y = 0;
                    player.grounded = true;
                }
            }

            if (lastPlayerRect.y >= wallRect.y + wallRect.h)
            {
                // If the sprite is now inside the platform
                if (playerRect.y < wallRect.y + wallRect.h)
                {
                    // Move the sprite down
                    player.sprite.pos.y = wallRect.y + wallRect.h;
                    player.velocity.y = 0;
                }
            }

            // Update the player rect
            playerRect.x = player.sprite.pos.x;
            playerRect.y = player.sprite.pos.y;

            // If we're still colliding, try to resolve the X axis
            if (!SDL_IntersectRect(&playerRect, &wallRect, &overlap))
            {
                continue;
            }

            // If the sprite was previous to the left of the platform
            if (lastPlayerRect.x + lastPlayerRect.w <= wallRect.x)
            {
                // If the sprite is now inside the platform
                if (playerRect.x + playerRect.w > wallRect.x)
                {
                    // Move the sprite left
                    player.sprite.pos.x = wallRect.x - playerRect.w;
                    player.velocity.x = 0;
                }
            }

            if (lastPlayerRect.x >= wallRect.x + wallRect.w)
            {
                // WTF IS THIS 0.5??? Is it the float to integer casting?
                // If the sprite is now inside the platform
                if (playerRect.x <= wallRect.x + wallRect.w + 0.5)
                {
                    // Move the sprite right
                    player.sprite.pos.x = wallRect.x + wallRect.w + 0.5;
                    player.velocity.x = 0;
                }
            }
        }

        // Handle coin collisions
        for (int i = 0; i < coinList.count; i++)
        {
            Coin *coin = EntityListGetEntity(&coinList, i + 1);

            if (coin->delete)
            {
                EntityListRemoveAtIndex(&coinList, i);
                continue;
            }

            if (IsCollision(&player.sprite, &coin->sprite))
            {
                CoinCollect(coin);
            }
        }

        lastPlayerRect = playerRect;

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 128, 200, 255);
        SDL_RenderClear(renderer);

        // Draw the coins
        for (int i = 0; i < coinList.count; i++)
        {
            // TODO(SeedyROM): This data should be iterated over the actual memory block
            // instead of the references.
            Coin *coin = EntityListGetEntity(&coinList, i + 1);
            CoinDraw(coin, renderer);
            CoinUpdate(coin);
        }

        // Draw the player
        PlayerDraw(&player, renderer);

        // Draw the walls
        for (int i = 0; i < wallCount; i++)
        {
            wallSprite.pos = walls[i].position;
            SpriteDraw(&wallSprite, renderer);
        }

        // Update the screen
        SDL_RenderPresent(renderer);

        // 60 FPS
        SDL_Delay(16);

        time++;
    }

    // Clear the coins entity list
    EntityListClear(&coinList);

    // Free the texture atlas
    TextureAtlasFree(textureAtlas);

    // Close the controller
    if (controller != NULL)
    {
        SDL_GameControllerClose(controller);
    }

    // Shutdown SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // Clean up memory
    ArenaFree(arena);

    return 0;
}
