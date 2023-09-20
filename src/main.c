#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "arena.h"
#include "entity.h"
#include "gfx.h"
#include "str.h"
#include "util.h"

static int windowWidth = 0;
static int windowHeight = 0;
static f32 gravity = 0.05f;

typedef struct Controllable
{
    Vec2 *position;
    Vec2 *velocity;

    void (*update)(struct Controllable *controllable, SDL_GameController *controller);
} Controllable;

void ControllableInit(Controllable *controllable, Vec2 *position, Vec2 *velocity, void (*update)(Controllable *controllable, SDL_GameController *controller))
{
    controllable->position = position;
    controllable->velocity = velocity;
    controllable->update = update;
}

void ControllableUpdate(Controllable *controllable, SDL_GameController *controller)
{
    controllable->update(controllable, controller);
}

typedef struct Player
{
    Sprite sprite;
    Vec2 velocity;
    bool grounded;
} Player;

void PlayerInit(Player *player, TextureAtlas *atlas, String *name)
{
    SpriteFromAtlas(&player->sprite, atlas, name);
    player->velocity = (Vec2){0, 0};
    player->grounded = false;
}

void PlayerControl(Controllable *controllable, SDL_GameController *controller)
{
    bool isUpPressed = false;

    Vec2 *velocity = controllable->velocity;

    const f32 maxSpeed = 2.0;
    // Move with arrow keys
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // TODO(SeedyROM): Dedup player control logic for keyboard and joystick
    // Move with joystick
    if (controller != NULL)
    {
        // Update the game controller
        SDL_GameControllerUpdate();

        // Get the joystick state
        Sint16 xAxis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 yAxis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        // printf("xAxis: %d, yAxis: %d\n", xAxis, yAxis);
        // printf("A Button: %d\n", SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A));

        // Deadzone
        if (xAxis > -8000 && xAxis < 8000)
        {
            xAxis = 0;
        }
        if (yAxis > -8000 && yAxis < 8000)
        {
            yAxis = 0;
        }

        // Move
        if (xAxis < 0)
        {
            // Set to 0 if facing the other way
            if (velocity->x > 0)
                velocity->x = 0;

            if (velocity->x > -maxSpeed)
                velocity->x -= 0.05f;
            else if (velocity->x < -maxSpeed)
                velocity->x = -maxSpeed;
        }
        if (xAxis > 0)
        {
            // Set to 0 if facing the other way
            if (velocity->x < 0)
                velocity->x = 0;

            if (velocity->x < maxSpeed)
                velocity->x += 0.05f;
            else if (velocity->x > maxSpeed)
                velocity->x = maxSpeed;
        }

        // If the gravity is zero allow up and down movement
        if (gravity == 0)
        {
            if (yAxis < 0)
            {
                velocity->y = -1.2;
            }
            if (yAxis > 0)
            {
                velocity->y = 1.2;
            }

            // If up and down are not pressed, slow down
            if (yAxis == 0)
            {
                if (velocity->y > 0)
                {
                    velocity->y -= 0.09f;
                    if (velocity->y < 0)
                        velocity->y = 0;
                }
                else if (velocity->y < 0)
                {
                    velocity->y += 0.09f;
                    if (velocity->y > 0)
                        velocity->y = 0;
                }
            }
        }

        // If left and right are not pressed, slow down
        if (xAxis == 0)
        {
            if (velocity->x > 0)
            {
                velocity->x -= 0.09f;
                if (velocity->x < 0)
                    velocity->x = 0;
            }
            else if (velocity->x < 0)
            {
                velocity->x += 0.09f;
                if (velocity->x > 0)
                    velocity->x = 0;
            }
        }

        // Jump
        // if (yAxis < 0)
        // {
        isUpPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);

        if (isUpPressed)
        {
            velocity->y = -1.4;
            isUpPressed = false;
        }
        // }
    }
    else
    {

        if (state[SDL_SCANCODE_LEFT])
        {
            if (velocity->x > -maxSpeed)
                velocity->x -= 0.05f;
            else if (velocity->x < -maxSpeed)
                velocity->x = -maxSpeed;
        }
        if (state[SDL_SCANCODE_RIGHT])
        {
            if (velocity->x < maxSpeed)
                velocity->x += 0.05f;
            else if (velocity->x > maxSpeed)
                velocity->x = maxSpeed;
        }

        // If left and right are not pressed, slow down
        if (!state[SDL_SCANCODE_LEFT] && !state[SDL_SCANCODE_RIGHT])
        {
            if (velocity->x > 0)
            {
                velocity->x -= 0.09f;
                if (velocity->x < 0)
                    velocity->x = 0;
            }
            else if (velocity->x < 0)
            {
                velocity->x += 0.09f;
                if (velocity->x > 0)
                    velocity->x = 0;
            }
        }

        // Jump
        if (state[SDL_SCANCODE_UP])
        {
            velocity->y = -1.4;
        }
    }
}

void PlayerUpdate(Player *player)
{
    Sprite *sprite = &player->sprite;
    Vec2 *velocity = &player->velocity;

    // Gravity
    if (!player->grounded)
        velocity->y += gravity;

    // Flip the sprite
    if (player->velocity.x < 0)
    {
        sprite->flipX = true;
    }
    else if (player->velocity.x > 0)
    {
        sprite->flipX = false;
    }

    // Update position
    sprite->pos.x += player->velocity.x;
    sprite->pos.y += player->velocity.y;

    // TODO(SeedyROM: Clamp position to screen, REMOVE ME
    if (sprite->pos.x < 0)
    {
        sprite->pos.x = 0;
        player->velocity.x = 0;
    }
    if (sprite->pos.x > windowWidth - sprite->frames.ptr[sprite->currentFrame].w)
    {
        sprite->pos.x = windowWidth - sprite->frames.ptr[sprite->currentFrame].w;
        player->velocity.x = 0;
    }

    // Clamp position to screen y
    if (sprite->pos.y < 0)
    {
        sprite->pos.y = 0;
        player->velocity.y = 0;
    }
    if (sprite->pos.y > windowHeight - sprite->frames.ptr[sprite->currentFrame].h)
    {
        sprite->pos.y = windowHeight - sprite->frames.ptr[sprite->currentFrame].h;
    }
}

void PlayerDraw(Player *player, SDL_Renderer *renderer)
{
    SpriteDraw(&player->sprite, renderer);
}

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

Vec2 CollisionOverlap(Sprite *a, SDL_Rect *b)
{
    SDL_Rect aRect = a->frames.ptr[a->currentFrame];
    aRect.x = a->pos.x;
    aRect.y = a->pos.y;

    Vec2 nullResult = (Vec2){0.0f, 0.0f};
    float xOverlap = 0.0f, yOverlap = 0.0f;

    // Test X direction.
    if (aRect.x + aRect.w < b->x || b->x + b->w < aRect.x)
    {
        return nullResult;
    }
    else
    {
        // get center X's of this and other rectangle
        float thisCenterX = aRect.x + aRect.w / 2;
        float otherCenterX = b->x + b->w / 2;

        if (thisCenterX < otherCenterX)
        {
            xOverlap = (aRect.x + aRect.w - b->x);
        }
        else
        {
            xOverlap = (b->x + b->w - aRect.x) * -1;
        }
    }

    // Test Y direction.
    if (aRect.y + aRect.h < b->y || b->y + b->h < aRect.y)
    {
        return nullResult;
    }
    else
    {
        // get center Y's of this and other rectangle
        float thisCenterY = aRect.y + aRect.h / 2;
        float otherCenterY = b->y + b->h / 2;

        if (thisCenterY < otherCenterY)
        {
            yOverlap = (aRect.y + aRect.h - b->y);
        }
        else
        {
            yOverlap = (b->y + b->h - aRect.y) * -1;
        }
    }

    return (Vec2){xOverlap, yOverlap};
}

typedef struct Coin
{
    TextureAtlas *atlas;
    bool collected;
    Sprite sprite;
    usize time;
    usize collectedTime;
    u16 frameDuration;
    u16 currentFrame;
    bool delete;
} Coin;

void CoinInit(Coin *coin, TextureAtlas *atlas)
{
    coin->collected = false;
    coin->atlas = atlas;
    coin->time = 0;
    coin->collectedTime = 0;
    coin->frameDuration = 10;
    coin->currentFrame = 0;
    coin->delete = false;

    SpriteFromAtlas(&coin->sprite, atlas, &STR("coin"));
}

void CoinCollect(Coin *coin)
{
    coin->collected = true;

    Vec2 pos = coin->sprite.pos;
    SpriteFromAtlas(&coin->sprite, coin->atlas, &STR("coin_collected"));
    coin->sprite.pos = pos;
    coin->currentFrame = 0;

    coin->frameDuration = 5;
    coin->time = 5;
}

void CoinDraw(Coin *coin, SDL_Renderer *renderer)
{
    SpriteDrawFrame(&coin->sprite, renderer, coin->currentFrame);
}

void CoinUpdate(Coin *coin)
{
    if (coin->collected)
    {
        coin->collectedTime += 1;
        if (coin->collectedTime > 30)
        {
            coin->delete = true;
        }
    }
    if (coin->time % coin->frameDuration == 0)
    {
        coin->currentFrame = (coin->currentFrame + 1) % coin->sprite.frames.len;
    }
    coin->time += 1;
}

typedef struct Wall
{
    Vec2 position;
} Wall;

void WallInit(Wall *wall, Vec2 position)
{
    wall->position = position;
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

    Player player;
    PlayerInit(&player, textureAtlas, &STR("capy_idle"));

    // Setup the player
    Controllable playerControl;
    ControllableInit(&playerControl, &player.sprite.pos, &player.velocity, &PlayerControl);

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
        {1, 0, 1, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1},
        {1, 2, 2, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1},
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

    // // Add some coins
    // for (int i = 0; i < 32; i++)
    // {
    //     // Create a coin sprite, don't alloc.
    //     Coin coin;
    //     CoinInit(&coin, textureAtlas);

    //     // Put a coin randomly on the screen
    //     coin.sprite.pos.x = rand() % windowWidth;
    //     coin.sprite.pos.y = rand() % windowHeight;

    //     EntityListAdd(&coinList, &coin);
    // }

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
        PlayerUpdate(&player);

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
            else if (lastPlayerRect.y >= wallRect.y + wallRect.h)
            {
                // If the sprite is now inside the platform
                if (playerRect.y < wallRect.y + wallRect.h)
                {
                    // Move the sprite down
                    player.sprite.pos.y = wallRect.y + wallRect.h;
                    player.velocity.y = 0;
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
