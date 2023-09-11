#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef struct Arena
{
    void *base;
    usize size;
    usize used;
} Arena;

Arena *ArenaAlloc(usize size);
void ArenaFree(Arena *arena);

void *ArenaPush(Arena *arena, usize size);
void *ArenaPushZero(Arena *arena, usize count, usize size);
void ArenaPop(Arena *arena, usize size);

usize ArenaGetPosition(Arena *arena);
void ArenaSetPositionBack(Arena *arena, usize position);

void ArenaClear(Arena *arena);

#define ArenaPushStruct(arena, type) (type *)ArenaPush(arena, sizeof(type))
#define AreanPushStructZero(arena, type) (type *)ArenaPushZero(arena, sizeof(type))
#define ArenaPushArray(arena, count, type) (type *)ArenaPush(arena, (count) * sizeof(type))
#define ArenaPushArrayZero(arena, count, type) (type *)ArenaPushZero(arena, (count) * sizeof(type))
#define ArenaPopStruct(arena, type) ArenaPop(arena, sizeof(type))
#define ArenaPopArray(arena, count, type) ArenaPop(arena, (count) * sizeof(type))

typedef struct TempMemory
{
    Arena *arena;
    usize position;
} TempMemory;

#define ArenaBeginTemp(arena) TempMemory _temp = {arena, ArenaGetPosition(arena)}
#define ArenaEndTemp(_arena) ArenaSetPositionBack(_arena, _temp.position), _temp.arena = NULL

#define tempMemoryBlock(_arena) for (ArenaBeginTemp(_arena); _temp.arena; ArenaEndTemp(_arena))