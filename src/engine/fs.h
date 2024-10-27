#pragma once

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "engine/arena.h"
#include "engine/str.h"
#include "engine/util.h"

typedef struct ByteArray {
  usize len;
  u8 *ptr;
} ByteArray;

usize GetFileSize(FILE *file);
String *ReadFileString(Arena *arena, String *path);
ByteArray *ReadFileBytes(Arena *arena, String *path);

void *ByteArrayReadArray(ByteArray *array, usize size, usize *offset,
                         usize count);

// ========================================================================================
// Single byte read macros
// ========================================================================================

#define ByteArrayRead(array, type, offset, count)                              \
  (type *)ByteArrayReadArray(array, sizeof(type), offset, count)

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

// ========================================================================================
// Array read macros
// ========================================================================================

#define ByteArrayReadArrayU8(array, offset, count)                             \
  ByteArrayRead(array, u8, offset, count)
#define ByteArrayReadArrayU16(array, offset, count)                            \
  ByteArrayRead(array, u16, offset, count)
#define ByteArrayReadArrayU32(array, offset, count)                            \
  ByteArrayRead(array, u32, offset, count)
#define ByteArrayReadArrayU64(array, offset, count)                            \
  ByteArrayRead(array, u64, offset, count)

#define ByteArrayReadArrayI8(array, offset, count)                             \
  ByteArrayRead(array, i8, offset, count)
#define ByteArrayReadArrayI16(array, offset, count)                            \
  ByteArrayRead(array, i16, offset, count)
#define ByteArrayReadArrayI32(array, offset, count)                            \
  ByteArrayRead(array, i32, offset, count)
#define ByteArrayReadArrayI64(array, offset, count)                            \
  ByteArrayRead(array, i64, offset, count)

#define ByteArrayReadArrayF32(array, offset, count)                            \
  ByteArrayRead(array, f32, offset, count)
#define ByteArrayReadArrayF64(array, offset, count)                            \
  ByteArrayRead(array, f64, offset, count)
