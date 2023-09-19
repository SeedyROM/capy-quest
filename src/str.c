#include "str.h"

String *StringCopy(Arena *arena, String *string)
{
    String *result = ArenaPushStruct(arena, String);
    result->len = string->len;
    result->ptr = ArenaPushArray(arena, result->len + 1, char);
    memcpy(result->ptr, string->ptr, result->len + 1);
    return result;
}

String *StringCopyCString(Arena *arena, const char *string)
{
    String *result = ArenaPushStruct(arena, String);
    result->len = strlen(string);
    result->ptr = ArenaPushArray(arena, result->len + 1, char);
    memcpy(result->ptr, string, result->len + 1);
    return result;
}

void StringSlice(String *string, u64 start, u64 end)
{
    string->ptr += start;
    string->len = end - start;

    // Null-terminate the string for convenience
    string->ptr[string->len + 1] = '\0';
}

u64 StringFindLastOccurrence(String *string, char c)
{
    for (u64 i = string->len; i > 0; i--)
    {
        if (string->ptr[i] == c)
        {
            return i;
        }
    }

    return 0;
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

int StringCompare(String *string1, String *string2)
{
    return strncmp(string1->ptr, string2->ptr, string2->len);
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
