#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "std.h"

static f32 gravity = 0.05f;

bool IsCollision(Sprite *a, Sprite *b) {
    SDL_Rect aRect = a->frames.ptr[a->currentFrame];
    SDL_Rect bRect = b->frames.ptr[b->currentFrame];

    // Use the center of the sprite for collision detection
    aRect.x = a->pos.x + (aRect.w / 2);
    aRect.y = a->pos.y + (aRect.h / 2);
    bRect.x = b->pos.x + (bRect.w / 2);
    bRect.y = b->pos.y + (bRect.h / 2);

    if (aRect.x + aRect.w < bRect.x || bRect.x + bRect.w < aRect.x) {
        return false;
    }

    if (aRect.y + aRect.h < bRect.y || bRect.y + bRect.h < aRect.y) {
        return false;
    }

    return true;
}

int main(void) {
    Arena *globalArena = ArenaAlloc(128 * Megabyte);

    // Initialize the game
    Game game;
    if (GameInit(&game) != 0) {
        printf("Failed to initialize capy-quest\n");
        return 1;
    }

    // Load the default controller
    if (GameLoadDefaultController(&game) != 0) {
        printf("Failed to load default controller\n");
        printf("Will use keyboard controls instead\n");
    }

    // Get the window, renderer and controller.
    SDL_Renderer *renderer = game.renderer;
    SDL_GameController *controller = game.controller;

    // Load the texture atlas from the assets folder
    TextureAtlas *textureAtlas = TextureAtlasCreate(globalArena);
    TextureAtlasLoadSprites(renderer, textureAtlas, &STR("../assets/sprites/*.aseprite"));

    // Create the camera and set the position
    Camera camera;
    camera.position = (Vec2){0, 0};
    camera.scale = (Vec2){1, 1};
    camera.rotation = 0;

    // Get the capy sprite
    Sprite capySprite;
    SpriteFromAtlas(&capySprite, textureAtlas, &STR("capy_idle"));

    // Setup the player
    Player player;
    PlayerInit(&player, &capySprite);

    // Setup the player control
    Controllable playerControl;
    ControllableInit(&playerControl, &player.sprite.pos, &player.velocity, &player.grounded, &PlayerControl);

    // Coin entity list
    EntityList coinList;
    EntityListInit(globalArena, &coinList, sizeof(Coin), 32);

    // Get the wall sprite
    Sprite wallSprite;
    SpriteFromAtlas(&wallSprite, textureAtlas, &STR("rock"));

    // Create a simple map!
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

    // Create the walls
    Wall walls[128];
    int wallCount = 0;

    // Add the objects to the map
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 16; x++) {
            int tile = map[y][x];
            if (tile == 1) {
                Wall wall;
                WallInit(&wall, (Vec2){x * 16, y * 16});
                walls[wallCount] = wall;
                wallCount++;
            }

            if (tile == 2) {
                // Create a coin sprite, don't alloc.
                Coin coin;
                CoinInit(&coin, textureAtlas);

                // Put a coin randomly on the screen
                coin.sprite.pos.x = x * 16 + 4;
                coin.sprite.pos.y = y * 16 + 4;

                EntityListAdd(&coinList, &coin);
            }

            if (tile == 3) {
                player.sprite.pos.x = x * 16;
                player.sprite.pos.y = y * 16;
            }
        }
    }

    // Store the last player rect
    SDL_Rect lastPlayerRect = {0, 0, 0, 0};

    // Dumb timer
    u64 time = 0;

    // Loop de loop
    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            // Quit this fucker
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // Add the controller if it's plugged in
            if (event.type == SDL_CONTROLLERDEVICEADDED) {
                printf("Attempting to add controller\n");
                controller = SDL_GameControllerOpen(0);
                if (controller == NULL) {
                    printf("Failed to add controller\n");
                    fprintf(stderr, "SDL_JoystickOpen Error: %s\n", SDL_GetError());
                }
            }

            // Remove the controller if it's unplugged
            if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
                printf("Controller disconnected\n");
                SDL_GameControllerClose(controller);
                controller = NULL;
            }
        }

        // This is awful but update the player sprite
        if (time % 20 == 0) {
            SpriteNextFrame(&player.sprite);
        }

        // Update the player
        ControllableUpdate(&playerControl, controller);
        PlayerUpdate(&player, gravity);

        // Handle collisions
        SDL_Rect playerRect = player.sprite.frames.ptr[player.sprite.currentFrame];
        playerRect.x = player.sprite.pos.x;
        playerRect.y = player.sprite.pos.y;

        // Resolve collisions with walls
        for (int i = 0; i < wallCount; i++) {
            Wall wall = walls[i];

            SDL_Rect wallRect = wallSprite.frames.ptr[wallSprite.currentFrame];
            wallRect.x = wall.position.x;
            wallRect.y = wall.position.y;

            SDL_Rect overlap = {0, 0, 0, 0};

            if (!SDL_IntersectRect(&playerRect, &wallRect, &overlap)) {
                player.grounded = false;
                continue;
            }

            // If the sprite was previous above the platform
            if (lastPlayerRect.y + lastPlayerRect.h <= wallRect.y) {
                // If the sprite is now inside the platform
                if (playerRect.y + playerRect.h > wallRect.y) {
                    // Move the sprite up
                    player.sprite.pos.y = wallRect.y - playerRect.h;
                    player.velocity.y = 0;
                    player.grounded = true;
                }
            }

            if (lastPlayerRect.y >= wallRect.y + wallRect.h) {
                // If the sprite is now inside the platform
                if (playerRect.y < wallRect.y + wallRect.h) {
                    // Move the sprite down
                    player.sprite.pos.y = wallRect.y + wallRect.h;
                    player.velocity.y = 0;
                }
            }

            // Update the player rect
            playerRect.x = player.sprite.pos.x;
            playerRect.y = player.sprite.pos.y;

            // If we're still colliding, try to resolve the X axis
            if (!SDL_IntersectRect(&playerRect, &wallRect, &overlap)) {
                continue;
            }

            // If the sprite was previous to the left of the platform
            if (lastPlayerRect.x + lastPlayerRect.w <= wallRect.x) {
                // If the sprite is now inside the platform
                if (playerRect.x + playerRect.w > wallRect.x) {
                    // Move the sprite left
                    player.sprite.pos.x = wallRect.x - playerRect.w;
                    player.velocity.x = 0;
                }
            }

            if (lastPlayerRect.x >= wallRect.x + wallRect.w) {
                // WTF IS THIS 0.5??? Is it the float to integer casting?
                // If the sprite is now inside the platform
                if (playerRect.x <= wallRect.x + wallRect.w + 0.5) {
                    // Move the sprite right
                    player.sprite.pos.x = wallRect.x + wallRect.w + 0.5;
                    player.velocity.x = 0;
                }
            }
        }

        // Handle coin collisions
        for (int i = 0; i < coinList.count; i++) {
            Coin *coin = EntityListGetEntity(&coinList, i + 1);
            CoinUpdate(coin);

            if (coin->delete) {
                EntityListRemoveAtIndex(&coinList, i);
                continue;
            }

            if (IsCollision(&player.sprite, &coin->sprite)) {
                CoinCollect(coin);
            }
        }

        lastPlayerRect = playerRect;

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 128, 200, 255);
        SDL_RenderClear(renderer);

        // Draw the coins
        for (int i = 0; i < coinList.count; i++) {
            // TODO(SeedyROM): This data should be iterated over the actual memory block
            // instead of the references.
            Coin *coin = EntityListGetEntity(&coinList, i + 1);
            SpriteDrawFrame(&coin->sprite, renderer, coin->currentFrame);
        }

        // Draw the player
        SpriteDraw(&player.sprite, renderer);

        // Draw the walls
        for (int i = 0; i < wallCount; i++) {
            wallSprite.pos = walls[i].position;
            SpriteDraw(&wallSprite, renderer);
        }

        // Update the screen
        SDL_RenderPresent(renderer);

        // 60 FPS
        SDL_Delay(16);

        // Time keeps on slipping, slipping, slipping...
        time++;
    }

    // Clear the coins entity list
    EntityListClear(&coinList);

    // Free the texture atlas
    TextureAtlasFree(textureAtlas);

    // Shutdown the game
    GameShutdown(&game);

    // Clean up memory
    ArenaFree(globalArena);

    return 0;
}
