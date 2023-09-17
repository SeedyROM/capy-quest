#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "arena.h"
#include "gfx.h"
#include "str.h"
#include "util.h"

static int windowWidth = 0;
static int windowHeight = 0;
static f32 gravity = 0.05f;

typedef struct Player
{
    Sprite *sprite;
    Vec2 velocity;
} Player;

Player *PlayerInit(Arena *arena, Player *player, TextureAtlas *atlas, String *name)
{
    player->sprite = SpriteFromAtlas(arena, atlas, name);
    player->velocity = (Vec2){0, 0};

    return player;
}

void PlayerUpdate(Player *player, SDL_GameController *controller)
{
    bool isUpPressed = false;

    // Gravity
    player->velocity.y += gravity;

    const f32 maxSpeed = 2.0;
    // Move with arrow keys
    const Uint8 *state = SDL_GetKeyboardState(NULL);

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
            if (player->velocity.x > 0)
                player->velocity.x = 0;

            if (player->velocity.x > -maxSpeed)
                player->velocity.x -= 0.05f;
            else if (player->velocity.x < -maxSpeed)
                player->velocity.x = -maxSpeed;
            player->sprite->flipX = true;
        }
        if (xAxis > 0)
        {
            // Set to 0 if facing the other way
            if (player->velocity.x < 0)
                player->velocity.x = 0;

            if (player->velocity.x < maxSpeed)
                player->velocity.x += 0.05f;
            else if (player->velocity.x > maxSpeed)
                player->velocity.x = maxSpeed;
            player->sprite->flipX = false;
        }

        // If left and right are not pressed, slow down
        if (xAxis == 0)
        {
            if (player->velocity.x > 0)
            {
                player->velocity.x -= 0.05f;
                if (player->velocity.x < 0)
                    player->velocity.x = 0;
            }
            else if (player->velocity.x < 0)
            {
                player->velocity.x += 0.05f;
                if (player->velocity.x > 0)
                    player->velocity.x = 0;
            }
        }

        // Jump
        // if (yAxis < 0)
        // {
        isUpPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);

        if (isUpPressed)
        {
            player->velocity.y = -1.2;
            isUpPressed = false;
        }
        // }
    }
    else
    {

        if (state[SDL_SCANCODE_LEFT])
        {
            if (player->velocity.x > -maxSpeed)
                player->velocity.x -= 0.05f;
            else if (player->velocity.x < -maxSpeed)
                player->velocity.x = -maxSpeed;
            player->sprite->flipX = true;
        }
        if (state[SDL_SCANCODE_RIGHT])
        {
            if (player->velocity.x < maxSpeed)
                player->velocity.x += 0.05f;
            else if (player->velocity.x > maxSpeed)
                player->velocity.x = maxSpeed;
            player->sprite->flipX = false;
        }

        // If left and right are not pressed, slow down
        if (!state[SDL_SCANCODE_LEFT] && !state[SDL_SCANCODE_RIGHT])
        {
            if (player->velocity.x > 0)
            {
                player->velocity.x -= 0.05f;
                if (player->velocity.x < 0)
                    player->velocity.x = 0;
            }
            else if (player->velocity.x < 0)
            {
                player->velocity.x += 0.05f;
                if (player->velocity.x > 0)
                    player->velocity.x = 0;
            }
        }

        // Jump
        if (state[SDL_SCANCODE_UP])
        {
            player->velocity.y = -1.2;
        }
    }

    // Update position
    player->sprite->pos.x += player->velocity.x;
    player->sprite->pos.y += player->velocity.y;

    // TODO(SeedyROM: Clamp position to screen, REMOVE ME
    if (player->sprite->pos.x < 0)
    {
        player->sprite->pos.x = 0;
    }
    if (player->sprite->pos.x > windowWidth - player->sprite->frames.ptr[player->sprite->currentFrame].w)
    {
        player->sprite->pos.x = windowWidth - player->sprite->frames.ptr[player->sprite->currentFrame].w;
    }

    // Clamp position to screen y
    if (player->sprite->pos.y < 0)
    {
        player->sprite->pos.y = 0;
    }
    if (player->sprite->pos.y > windowHeight - player->sprite->frames.ptr[player->sprite->currentFrame].h)
    {
        player->sprite->pos.y = windowHeight - player->sprite->frames.ptr[player->sprite->currentFrame].h;
    }
}
void PlayerDraw(Player *player, SDL_Renderer *renderer)
{
    SpriteDraw(player->sprite, renderer);
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

    // Load the texture atlas
    TextureAtlas *textureAtlas = TextureAtlasInit(arena);
    TextureAtlasLoadSprites(renderer, textureAtlas, &STR("../assets/sprites/*.aseprite"));

    Player player;
    PlayerInit(arena, &player, textureAtlas, &STR("capy"));

    // Make a coin sprite
    Sprite *coinSprite = SpriteFromAtlas(arena, textureAtlas, &STR("coin"));
    coinSprite->pos.x = 100;
    coinSprite->pos.y = 100;

    // Gravity
    f32 gravity = 0.05f;

    // Dumb timer
    u64 time = 0;

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

    i16 startX, startY;
    if (controller != NULL)
    {
        // Print the joystick name
        printf("Controller name: %s\n", SDL_GameControllerName(controller));
    }

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

        if (time % 20 == 0)
        {
            SpriteNextFrame(player.sprite);
        }

        if (time % 10 == 0)
        {
            SpriteNextFrame(coinSprite);
        }

        PlayerUpdate(&player, controller);

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 128, 200, 255);
        SDL_RenderClear(renderer);

        PlayerDraw(&player, renderer);
        SpriteDraw(coinSprite, renderer);

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
