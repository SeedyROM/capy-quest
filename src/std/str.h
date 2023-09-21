#pragma once

#include <stdarg.h>
#include <string.h>

#include "std/arena.h"
#include "std/util.h"

typedef struct String {
    u64 len;
    char *ptr;
} String;

String *StringCopy(Arena *arena, String *string);
String *StringCopyCString(Arena *arena, const char *string);
void StringSlice(String *string, u64 start, u64 end);
u64 StringFindLastOccurrence(String *string, char c);
int StringCompare(String *string1, String *string2);

#define STR(ptr) \
    (String) { sizeof(ptr) - 1, ptr }
#define STR_EQUAL(str1, str2) \
    (str1.len == str2.len && strncmp(str1.ptr, str2.ptr, min(str1.len, str2.len)) == 0)

typedef struct StringBuilder {
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
