#include "gfx.h"

#include <glob.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rectpack.h>

#include "aseprite.h"
#include "util.h"

TextureAtlas *TextureAtlasCreate(Arena *arena)
{
    TextureAtlas *atlas = ArenaPushStruct(arena, TextureAtlas);
    atlas->arena = arena;
    atlas->indices = ARRAY_INIT_DEFINED(atlas->arena, TextureAtlasIndices, TextureAtlasIndex, 128);
    atlas->frames = ARRAY_INIT_DEFINED(atlas->arena, TextureAtlasFrames, TextureAtlasFrame, 128);
    atlas->texture = NULL;
    atlas->width = 0;
    atlas->height = 0;

    return atlas;
}

int TextureAtlasLoadSprites(SDL_Renderer *renderer, TextureAtlas *atlas, String *path)
{
    Arena *scratch = ArenaAlloc(128 * Megabyte);
    tempMemoryBlock(scratch)
    {

        // Sprite assets memory
        typedef struct SpriteAssetPath
        {
            String name;
            String path;
        } SpriteAssetPath;
        ARRAY(SpriteAssetPath, spriteAssetPaths);

        // Glob for sprite assets
        {
            glob_t globResult;
            glob(path->ptr, GLOB_TILDE, NULL, &globResult);

            // Allocate space for the paths
            spriteAssetPaths.len = globResult.gl_pathc;
            spriteAssetPaths.ptr = ArenaPushArrayZero(scratch, spriteAssetPaths.len, SpriteAssetPath);

            printf("Found %zu sprite assets\n", spriteAssetPaths.len);

            // Get the paths;
            for (usize i = 0; i < spriteAssetPaths.len; i++)
            {
                // Copy each path from the glob
                String *pathString = StringCopyCString(scratch, globResult.gl_pathv[i]);
                spriteAssetPaths.ptr[i].path = *pathString;

                // Get the name of the sprite asset without the extension and path
                String *nameString = StringCopy(scratch, pathString);
                u64 lastSlash = StringFindLastOccurrence(nameString, '/') + 1;
                u64 lastDot = StringFindLastOccurrence(nameString, '.') - 1;
                StringSlice(nameString, lastSlash, lastDot);

                // Print the sprite asset name
                printf("Sprite asset: %s\n", nameString->ptr);

                // Put the name in the sprite asset path
                spriteAssetPaths.ptr[i].name = *nameString;
            }
        }

        // Load the sprite assets
        ARRAY_ALLOC_RESERVED(scratch, AsepriteFile, sprites, spriteAssetPaths.len);

        // Allocate space for the sprite frames
        ARRAY_ALLOC(scratch, AsepriteAnimationFrame, spriteFrames, 128);

        for (usize i = 0; i < sprites.len; i++)
        {
            // Read the sprite asset file
            String *spriteAssetPath = &spriteAssetPaths.ptr[i].path;
            String *spriteAssetName = &spriteAssetPaths.ptr[i].name;

            printf("Loading sprite asset: %s = %s\n", spriteAssetName->ptr, spriteAssetPath->ptr);
            AsepriteFile *sprite = &sprites.ptr[i];
            sprite = AsepriteLoad(scratch, spriteAssetPath);

            // Add the sprite asset to the atlas index
            TextureAtlasIndex atlasIndex = {
                .numFrames = sprite->numFrames,
                .frameIndex = spriteFrames.len,
                .name = StringCopy(atlas->arena, spriteAssetName)};
            ARRAY_PUSH(atlas->arena, atlas->indices, TextureAtlasIndex, atlasIndex);

            // Get all the the frames
            for (usize j = 0; j < sprite->numFrames; j++)
            {
                AsepriteAnimationFrame spriteFrameProcessed;
                AsepriteGetAnimationFrame(sprite, j, &spriteFrameProcessed);
                ARRAY_PUSH(scratch, spriteFrames, AsepriteAnimationFrame, spriteFrameProcessed);
            }
        }

        ARRAY_ALLOC_RESERVED(scratch, stbrp_rect, spriteRects, spriteFrames.len);
        for (usize i = 0; i < spriteFrames.len; i++)
        {
            stbrp_rect *rect = &spriteRects.ptr[i];
            rect->id = i;
            rect->w = spriteFrames.ptr[i].sizeX;
            rect->h = spriteFrames.ptr[i].sizeY;
        }

        // Pack the sprite frames using stb_rectpack
        {
            stbrp_context context;
            stbrp_node *nodes = ArenaPushArray(scratch, spriteFrames.len, stbrp_node);
            stbrp_init_target(&context, spriteRects.len * 8, INT32_MAX, nodes, spriteRects.len);
            int result = stbrp_pack_rects(&context, spriteRects.ptr, spriteRects.len);
            if (result == 0)
            {
                printf("Failed to pack sprite frames\n");
                return 1;
            }
        }

        // Get the size of the atlas
        for (usize i = 0; i < spriteFrames.len; i++)
        {
            stbrp_rect *rect = &spriteRects.ptr[i];
            if (rect->x + rect->w > atlas->width)
            {
                atlas->width = rect->x + rect->w;
            }

            if (rect->y + rect->h > atlas->height)
            {
                atlas->height = rect->y + rect->h;
            }
        }

        // Allocate space for the atlas
        u32 *atlasPixels = ArenaPushArrayZero(scratch, atlas->width * atlas->height, u32);

        // Copy the sprite frames into the atlas
        for (usize i = 0; i < spriteFrames.len; i++)
        {
            // Get the rect for the sprite frame
            stbrp_rect *rect = &spriteRects.ptr[i];

            // Push the rect to the atlas rects
            {
                SDL_Rect atlasRect = {rect->x, rect->y, rect->w, rect->h};
                ARRAY_PUSH(atlas->arena, atlas->frames, SDL_Rect, atlasRect);
            }

            // Get the frame pixels and copy them into the atlas
            AsepriteAnimationFrame *spriteFrame = &spriteFrames.ptr[rect->id];
            for (u16 y = 0; y < spriteFrame->sizeY; y++)
            {
                for (u16 x = 0; x < spriteFrame->sizeX; x++)
                {
                    u32 *atlasPixel = &atlasPixels[(rect->y + y) * atlas->width + (rect->x + x)];
                    u32 *spritePixel = &spriteFrame->pixels[y * spriteFrame->sizeX + x];

                    *atlasPixel = *spritePixel;
                }
            }
        }

        // Create a texture from the atlas
        atlas->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, atlas->width, atlas->height);

        // Copy the atlas pixels into the texture
        SDL_UpdateTexture(atlas->texture, NULL, atlasPixels, atlas->width * sizeof(u32));

        // Allow for alpha blending
        SDL_SetTextureBlendMode(atlas->texture, SDL_BLENDMODE_BLEND);
    }
    ArenaFree(scratch);

    return 0;
}

i64 TextureAtlasIndicesGetIndex(TextureAtlas *atlas, String *name)
{
    for (usize i = 0; i < atlas->indices.len; i++)
    {
        if (StringCompare(atlas->indices.ptr[i].name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}

TextureAtlasFrames TextureAtlasIndicesGetFrames(TextureAtlas *atlas, String *name)
{
    int index = TextureAtlasIndicesGetIndex(atlas, name);
    if (index < 0)
    {
        printf("Failed to find texture atlas index for %s\n", name->ptr);
        exit(EXIT_FAILURE);
    }

    TextureAtlasIndex *indexEntry = &atlas->indices.ptr[index];
    TextureAtlasFrames foundFrames = {
        .ptr = &atlas->frames.ptr[indexEntry->frameIndex],
        .len = indexEntry->numFrames};

    return foundFrames;
}

Sprite *SpriteFromAtlas(Arena *arena, TextureAtlas *atlas, String *name)
{
    Sprite *sprite = ArenaPushStruct(arena, Sprite);
    sprite->atlas = atlas;
    sprite->frames = TextureAtlasIndicesGetFrames(atlas, name);
    sprite->currentFrame = 0;
    sprite->pos = (Vec2){0, 0};
    sprite->scale = (Vec2){1, 1};
    sprite->rotation = 0;
    sprite->flipX = false;
    sprite->flipY = false;

    return sprite;
}

void SpriteDraw(Sprite *sprite, SDL_Renderer *renderer)
{
    SDL_Rect *frame = &sprite->frames.ptr[sprite->currentFrame];

    SDL_Rect destRect = {
        .x = sprite->pos.x,
        .y = sprite->pos.y,
        .w = frame->w * sprite->scale.x,
        .h = frame->h * sprite->scale.y};

    SDL_Point center = {
        .x = frame->w / 2,
        .y = frame->h / 2};

    SDL_RendererFlip flip = 0;
    if (sprite->flipX)
    {
        flip |= SDL_FLIP_HORIZONTAL;
    }

    if (sprite->flipY)
    {
        flip |= SDL_FLIP_VERTICAL;
    }

    SDL_RenderCopyEx(renderer, sprite->atlas->texture, frame, &destRect, sprite->rotation, &center, flip);
}

void SpriteNextFrame(Sprite *sprite)
{
    sprite->currentFrame = (sprite->currentFrame + 1) % sprite->frames.len;
}

void SpritePreviousFrame(Sprite *sprite)
{
    sprite->currentFrame = (sprite->currentFrame - 1) % sprite->frames.len;
}
