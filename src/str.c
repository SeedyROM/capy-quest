#include "str.h"

String *StringCopyCString(Arena *arena, const char *string)
{
    String *result = ArenaPushStruct(arena, String);
    result->len = strlen(string);
    result->ptr = ArenaPushArray(arena, result->len + 1, char);
    memcpy(result->ptr, string, result->len + 1);
    return result;
}

StringBuilder *StringBuilderAlloc(Arena *arena)
{
    StringBuilder *result = ArenaPushStruct(arena, StringBuilder);
    result->arena = arena;
    result->string = STR("");
    return result;
}

void StringBuilderAppend(StringBuilder *sb, String string)
{
    u64 newLen = sb->string.len + string.len;
    char *newPtr = ArenaPushArray(sb->arena, newLen + 1, char); // +1 for the null terminator
    memcpy(newPtr, sb->string.ptr, sb->string.len);
    memcpy(newPtr + sb->string.len, string.ptr, string.len);
    newPtr[newLen] = '\0'; // Null-terminate the concatenated string
    sb->string.len = newLen;
    sb->string.ptr = newPtr;
}

void StringBuilderClear(StringBuilder *sb)
{
    ArenaPop(sb->arena, sb->string.len);
    sb->string = STR("");
}

void StringBuilderFree(StringBuilder *sb)
{
    ArenaPopArray(sb->arena, sb->string.len, char);
    ArenaPopStruct(sb->arena, StringBuilder);
}
