#pragma once

#include "engine/util.h"

typedef struct Wall {
  Vec2 position;
} Wall;

void WallInit(Wall *wall, Vec2 position);
