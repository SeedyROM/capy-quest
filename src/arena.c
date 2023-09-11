#include "arena.h"

Arena *ArenaAlloc(usize size)
{
    Arena *arena = malloc(sizeof(Arena));

    arena->size = size;
    arena->base = malloc(arena->size);
    arena->used = 0;

    return arena;
}

void ArenaFree(Arena *arena)
{
    free(arena->base);
    free(arena);
}

void *ArenaPush(Arena *arena, usize size)
{
    if (arena->used + size > arena->size)
    {
        printf("Arena overflow\n");
        exit(1);
    }
    void *result = (u8 *)arena->base + arena->used;
    arena->used += size;
    return result;
}

void *ArenaPushZero(Arena *arena, usize count, usize size)
{
    void *result = ArenaPush(arena, count * size);
    memset(result, 0, count * size);
    return result;
}

void ArenaPop(Arena *arena, usize size)
{
    if (arena->used < size)
    {
        printf("Arena underflow\n");
        exit(1);
    }
    arena->used -= size;
}

usize ArenaGetPosition(Arena *arena)
{
    return arena->used;
}

void ArenaSetPositionBack(Arena *arena, usize position)
{
    if (position > arena->used)
    {
        printf("Arena underflow\n");
        exit(1);
    }
    arena->used = position;
}

void ArenaClear(Arena *arena)
{
    arena->used = 0;
}
