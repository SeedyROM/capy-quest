#pragma once

#include <string.h>
#include <stdarg.h>

#include "arena.h"
#include "util.h"

typedef struct String
{
    u64 len;
    char *ptr;
} String;

String *StringCopyCString(Arena *arena, const char *string);

#define STR(ptr) \
    (String) { sizeof(ptr) - 1, ptr }
#define STR_EQUAL(str1, str2) \
    (str1.len == str2.len && strncmp(str1.ptr, str2.ptr, min(str1.len, str2.len)) == 0)

typedef struct StringBuilder
{
    Arena *arena;
    String string;
} StringBuilder;

StringBuilder *StringBuilderAlloc(Arena *arena);
void StringBuilderAppend(StringBuilder *sb, String string);
// TODO(SeedyROM): Implement StringBuilderFormat and StringBuilderAppendFormat
// void StringBuilderFormat(StringBuilder *sb, const char *format, va_list args);
// void StringBuilderAppendFormat(StringBuilder *sb, const char *format, va_list args);
void StringBuilderClear(StringBuilder *sb);
void StringBuilderFree(StringBuilder *sb);
