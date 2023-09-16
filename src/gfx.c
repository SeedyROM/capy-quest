#include "gfx.h"

i64 TextureAtlasIndicesGetIndex(TextureAtlasIndices *indices, String *name)
{
    for (usize i = 0; i < indices->len; i++)
    {
        if (StringCompare(indices->ptr[i].name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}

TextureAtlasFrames TextureAtlasIndicesGetFrames(TextureAtlasIndices *indices, TextureAtlasFrames *frames, String *name)
{
    int index = TextureAtlasIndicesGetIndex(indices, name);
    if (index < 0)
    {
        printf("Failed to find texture atlas index for %s\n", name->ptr);
        exit(EXIT_FAILURE);
    }

    TextureAtlasIndex *indexEntry = &indices->ptr[index];
    TextureAtlasFrames foundFrames = {
        .ptr = &frames->ptr[indexEntry->frameIndex],
        .len = indexEntry->numFrames};

    return foundFrames;
}
