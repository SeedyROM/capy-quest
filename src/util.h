#pragma once

#include <stdint.h>
#include <stddef.h>

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

#define ARRAY_ALLOC(arena, type, name, count) \
    ARRAY(type, name) = {                     \
        ArenaPushArray(arena, count, type),   \
        count,                                \
        count}
