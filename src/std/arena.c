#include "std/arena.h"

Arena *ArenaAlloc(usize size) {
    Arena *arena = malloc(sizeof(Arena));

    arena->size = size;
    arena->base = malloc(arena->size);
    arena->used = 0;

    return arena;
}

void ArenaFree(Arena *arena) {
    free(arena->base);
    free(arena);
}

void *ArenaPush(Arena *arena, usize size) {
    if (arena->used + size > arena->size) {
        printf("Arena overflow\n");
        exit(EXIT_FAILURE);
    }
    void *result = (u8 *)arena->base + arena->used;
    arena->used += size;
    return result;
}

void *ArenaPushZero(Arena *arena, usize size) {
    void *result = ArenaPush(arena, size);
    memset(result, 0, size);
    return result;
}

void ArenaPop(Arena *arena, usize size) {
    if (arena->used < size) {
        printf("Arena underflow\n");
        exit(EXIT_FAILURE);
    }
    arena->used -= size;
}

usize ArenaGetPosition(Arena *arena) {
    return arena->used;
}

void ArenaSetPositionBack(Arena *arena, usize position) {
    if (position > arena->used) {
        printf("Arena underflow\n");
        exit(EXIT_FAILURE);
    }
    arena->used = position;
}

void ArenaClear(Arena *arena) {
    arena->used = 0;
}
