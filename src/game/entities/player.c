#include "player.h"

void PlayerInit(Player *player, Sprite *sprite)
{
    player->sprite = *sprite;
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

        isUpPressed = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);

        if (isUpPressed)
        {
            velocity->y = -1.4;
            isUpPressed = false;
        }
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

void PlayerUpdate(Player *player, f32 gravity)
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
}
