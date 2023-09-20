#include "entity.h"

void EntityListInit(Arena *arena, EntityList *list, usize entitySize, u16 capacity)
{
    list->count = 0;
    list->capacity = capacity;
    list->refs = ArenaPushArray(arena, capacity, EntityRef);
    list->entitySize = entitySize;
    list->entities = ArenaPushArray(arena, entitySize * capacity, u8);
    list->lastEntity = list->entities;
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
    if (index < 0 || index >= list->count)
    {
        printf("EntityListRemoveAtIndex: index out of bounds\n");
        return;
    }

    EntityRef *ref = list->refs + index;
    EntityListRemove(list, ref);
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

void EntityListClear(EntityList *list)
{
    list->count = 0;
    list->lastEntity = list->entities;
}
