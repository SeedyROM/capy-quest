#pragma once

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "arena.h"

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

void EntityListInit(Arena *arena, EntityList *list, usize entitySize, u16 capacity);
EntityRef *EntityListAdd(EntityList *list, void *entity);
void EntityListRemove(EntityList *list, EntityRef *ref);
void EntityListRemoveAtIndex(EntityList *list, u16 index);
EntityRef *EntityListGet(EntityList *list, u16 id);
void *EntityListGetEntity(EntityList *list, u16 id);
void EntityListClear(EntityList *list);
