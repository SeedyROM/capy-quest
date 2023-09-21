#include "controllable.h"

void ControllableInit(Controllable *controllable, Vec2 *position, Vec2 *velocity, bool *grounded, void (*update)(Controllable *controllable, SDL_GameController *controller))
{
    controllable->position = position;
    controllable->velocity = velocity;
    controllable->grounded = grounded;
    controllable->update = update;
}

void ControllableUpdate(Controllable *controllable, SDL_GameController *controller)
{
    controllable->update(controllable, controller);
}
