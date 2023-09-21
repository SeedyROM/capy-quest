#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define Kilobyte (1024)
#define Megabyte (Kilobyte * 1024)
#define Gigabyte (Megabyte * 1024)

typedef size_t usize;

typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define ARRAY(type, name) \
    struct                \
    {                     \
        type *ptr;        \
        usize len;        \
        usize cap;        \
    } name

#define ARRAY_DEFINE(type, name) \
    typedef struct name {        \
        type *ptr;               \
        usize len;               \
        usize cap;               \
    } name

#define ARRAY_ALLOC_RESERVED(arena, type, name, count) \
    ARRAY(type, name) = {                              \
        ArenaPushArrayZero(arena, count, type),        \
        count,                                         \
        count}

#define ARRAY_ALLOC(arena, type, name, count)   \
    ARRAY(type, name) = {                       \
        ArenaPushArrayZero(arena, count, type), \
        0,                                      \
        count}

#define ARRAY_INIT(arena, type, subtype, name, count) \
    type name = {                                     \
        ArenaPushArrayZero(arena, count, subtype),    \
        0,                                            \
        count}

#define ARRAY_INIT_DEFINED(arena, type, subtype, count) \
    (type) {                                            \
        ArenaPushArrayZero(arena, count, subtype),      \
            0,                                          \
            count                                       \
    }

#define ARRAY_INIT_RESERVED(arena, type, subtype, name, count) \
    type name = {                                              \
        ArenaPushArrayZero(arena, count, subtype),             \
        count,                                                 \
        count}

// TODO(SeedyROM): Don't use memcpy here
#define ARRAY_PUSH(arena, array, type, value)                      \
    do {                                                           \
        if (array.len == array.cap) {                              \
            array.cap *= 2;                                        \
                                                                   \
            type *newPtr = ArenaPushArray(arena, array.cap, type); \
            memcpy(newPtr, array.ptr, array.len * sizeof(type));   \
            array.ptr = newPtr;                                    \
        }                                                          \
                                                                   \
        array.ptr[array.len++] = value;                            \
    } while (0)

typedef struct Vec2 {
    f32 x;
    f32 y;
} Vec2;
