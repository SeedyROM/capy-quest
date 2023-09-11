#include "fs.h"

usize GetFileSize(FILE *file)
{
    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return size;
}

String *ReadFileString(Arena *arena, String *path)
{
    FILE *file = fopen(path->ptr, "rb");
    if (!file)
    {
        printf("Failed to open file: %s\n", path->ptr);
        exit(1);
    }

    // Get the size of the file
    u64 size = GetFileSize(file);

    // Allocate space for the file
    String *result = ArenaPushStruct(arena, String);
    result->len = size;
    result->ptr = ArenaPushArray(arena, size + 1, char);

    // Read the file
    int bytesRead = fread(result->ptr, sizeof(char), size, file);
    if (bytesRead != size)
    {
        printf("Failed to read file: %s\n", path->ptr);
        exit(1);
    }

    // Null terminate the string
    result->ptr[size] = 0;

    return result;
}

ByteArray *ReadFileBytes(Arena *arena, String *path)
{
    FILE *file = fopen(path->ptr, "rb");
    if (!file)
    {
        printf("Failed to open file: %s\n", path->ptr);
        exit(1);
    }

    // Get the size of the file
    u64 size = GetFileSize(file);

    // Allocate space for the file
    ByteArray *result = ArenaPushStruct(arena, ByteArray);
    result->size = size;
    result->ptr = ArenaPushArray(arena, size, u8);

    // Read the file
    int bytesRead = fread(result->ptr, sizeof(u8), size, file);
    if (bytesRead != size)
    {
        printf("Failed to read file: %s\n", path->ptr);
        printf("Reason: %s\n", strerror(errno));
        exit(1);
    }

    fclose(file);
    return result;
}

void *ByteArrayReadArray(ByteArray *array, usize size, usize *offset, usize count)
{
    if (*offset + size * count > array->size)
    {
        printf("ByteArrayReadArray: Out of bounds\n");
        exit(1);
    }

    void *result = array->ptr + *offset;
    *offset += size * count;
    return result;
}