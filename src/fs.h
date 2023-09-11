#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "arena.h"
#include "util.h"
#include "str.h"

typedef struct ByteArray
{
    usize size;
    u8 *ptr;
} ByteArray;

usize GetFileSize(FILE *file);
String *ReadFileString(Arena *arena, String *path);
ByteArray *ReadFileBytes(Arena *arena, String *path);

void *ByteArrayReadArray(ByteArray *array, usize size, usize *offset, usize count);

#define ByteArrayRead(array, type, offset, count) (type *)ByteArrayReadArray(array, sizeof(type) * count, offset, count)

#define ByteArrayReadU8(array, offset) *ByteArrayRead(array, u8, offset, 1)
#define ByteArrayReadU16(array, offset) *ByteArrayRead(array, u16, offset, 1)
#define ByteArrayReadU32(array, offset) *ByteArrayRead(array, u32, offset, 1)
#define ByteArrayReadU64(array, offset) *ByteArrayRead(array, u64, offset, 1)

#define ByteArrayReadI8(array, offset) *ByteArrayRead(array, i8, offset, 1)
#define ByteArrayReadI16(array, offset) *ByteArrayRead(array, i16, offset, 1)
#define ByteArrayReadI32(array, offset) *ByteArrayRead(array, i32, offset, 1)
#define ByteArrayReadI64(array, offset) *ByteArrayRead(array, i64, offset, 1)

#define ByteArrayReadF32(array, offset) *ByteArrayRead(array, f32, offset, 1)
#define ByteArrayReadF64(array, offset) *ByteArrayRead(array, f64, offset, 1)