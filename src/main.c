#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "arena.h"
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
    Sprite *sprite;
    Vec2 velocity;
} Player;

Player *PlayerInit(Arena *arena, Player *player, TextureAtlas *atlas, String *name)
{
    player->sprite = SpriteFromAtlas(arena, atlas, name);
    player->velocity = (Vec2){0, 0};

    return player;
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
            velocity->y = -1.2;
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
            velocity->y = -1.2;
        }
    }
}

void PlayerUpdate(Player *player)
{
    Sprite *sprite = player->sprite;
    Vec2 *velocity = &player->velocity;

    // Gravity
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
    SpriteDraw(player->sprite, renderer);
}

typedef struct EntityRef
{
    u16 id;
    void *entity;
    struct EntityRef *next;
} EntityRef;

typedef struct EntityList
{
    u16 count;
    u16 capacity;
    EntityRef *refs;
    usize entitySize;
    void *entities;
    void *lastEntity;
} EntityList;

void EntityListInit(Arena *arena, EntityList *list, usize entitySize, u16 capacity)
{
    list->count = 0;
    list->capacity = capacity;
    list->refs = ArenaPushArray(arena, capacity, EntityRef);
    list->entitySize = entitySize;
    list->entities = ArenaPushArray(arena, entitySize * capacity, u8);
    list->lastEntity = list->entities;
}

void EntityListFree(EntityList *list)
{
    free(list->refs);
    free(list->entities);
}

EntityRef *EntityListAdd(EntityList *list, void *entity)
{
    if (list->count == list->capacity)
    {
        printf("EntityListAdd: list is full\n");
        return NULL;
    }

    // Add the reference to the list
    EntityRef *ref = list->refs + list->count;
    ref->id = list->count + 1;
    ref->entity = list->entities + (list->count * list->entitySize);
    ref->next = NULL;

    // Copy the entity into the list
    memcpy(ref->entity, entity, list->entitySize);

    list->lastEntity = entity;
    list->count++;

    return ref;
}

void EntityListRemove(EntityList *list, EntityRef *ref)
{
    if (ref->id == list->count)
    {
        list->count--;
        return;
    }

    EntityRef *lastRef = list->refs + list->count - 1;
    EntityRef *nextRef = ref->next;

    ref->id = lastRef->id;
    ref->entity = lastRef->entity;
    ref->next = nextRef;

    // Copy the last entity into the removed entity
    memcpy(ref->entity, lastRef->entity, list->entitySize);

    lastRef->id = 0;
    lastRef->entity = NULL;
    lastRef->next = NULL;

    list->count--;

    if (nextRef != NULL)
    {
        nextRef->id = ref->id;
    }

    if (list->count == 0)
    {
        list->lastEntity = list->entities;
    }
}

void EntityListRemoveAtIndex(EntityList *list, u16 index)
{
}

EntityRef *EntityListGet(EntityList *list, u16 id)
{
    if (id < 1 || id > list->count)
    {
        return NULL;
    }

    return list->refs + id - 1;
}

void *EntityListGetEntity(EntityList *list, u16 id)
{
    EntityRef *ref = EntityListGet(list, id);
    if (ref == NULL)
    {
        return NULL;
    }

    return ref->entity;
}

bool IsCollision(Sprite *a, Sprite *b)
{
    SDL_Rect aRect = a->frames.ptr[a->currentFrame];
    SDL_Rect bRect = b->frames.ptr[b->currentFrame];

    if (a->pos.x < b->pos.x + bRect.w &&
        a->pos.x + aRect.w > b->pos.x &&
        a->pos.y < b->pos.y + bRect.h &&
        a->pos.y + aRect.h > b->pos.y)
    {
        return true;
    }

    return false;
}

Vec2 CollisionOverlap(Sprite *a, Sprite *b)
{
    Vec2 overlap = {0, 0};

    SDL_Rect aRect = a->frames.ptr[a->currentFrame];
    SDL_Rect bRect = b->frames.ptr[b->currentFrame];

    if (a->pos.x < b->pos.x + bRect.w &&
        a->pos.x + aRect.w > b->pos.x &&
        a->pos.y < b->pos.y + bRect.h &&
        a->pos.y + aRect.h > b->pos.y)
    {
        overlap.x = a->pos.x - b->pos.x;
        overlap.y = a->pos.y - b->pos.y;
    }

    return overlap;
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
    int windowScaleFactor = 8;

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
    PlayerInit(arena, &player, textureAtlas, &STR("capy"));

    // Setup the player
    Controllable playerControl;
    ControllableInit(&playerControl, &player.sprite->pos, &player.velocity, &PlayerControl);

    // Coin entity list
    EntityList coinList;
    EntityListInit(arena, &coinList, sizeof(Sprite), 32);

    // Add some coins
    for (int i = 0; i < 32; i++)
    {
        Sprite *coinSprite = SpriteFromAtlas(arena, textureAtlas, &STR("coin"));
        Sprite *coin = EntityListAdd(&coinList, coinSprite)->entity;

        // Put a coin randomly on the screen
        coin->pos.x = rand() % windowWidth;
        coin->pos.y = rand() % windowHeight;
    }

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
            SpriteNextFrame(player.sprite);
        }

        // Update the player
        ControllableUpdate(&playerControl, controller);
        PlayerUpdate(&player);

        // Handle coin collisions
        for (int i = 0; i < coinList.count; i++)
        {
            Sprite *coin = EntityListGetEntity(&coinList, i + 1);
            if (IsCollision(player.sprite, coin))
            {
                EntityListRemoveAtIndex(&coinList, i);
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 128, 200, 255);
        SDL_RenderClear(renderer);

        // Draw the coins
        for (int i = 0; i < coinList.count; i++)
        {
            Sprite *coin = EntityListGetEntity(&coinList, i + 1);
            if (time % 10 == 0)
            {
                SpriteNextFrame(coin);
            }
            SpriteDraw(coin, renderer);
        }

        PlayerDraw(&player, renderer);

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
