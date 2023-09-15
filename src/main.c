#include <stdio.h>
#include <stdbool.h>
#include <glob.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rectpack.h>

#include "arena.h"
#include "aseprite.h"
#include "fs.h"
#include "str.h"
#include "util.h"

DEFINE_ARRAY(SDL_Rect, TextureAtlasFrames);

typedef struct TextureAtlasIndex
{
    u16 numFrames;
    u32 frameIndex;
    String *name;
} TextureAtlasIndex;
DEFINE_ARRAY(TextureAtlasIndex, TextureAtlasIndices);

i64 TextureAtlasIndicesGetIndex(TextureAtlasIndices *indices, String *name)
{
    for (usize i = 0; i < indices->len; i++)
    {
        if (StringCompare(indices->ptr[i].name, name))
        {
            return i;
        }
    }

    return -1;
}

TextureAtlasFrames TextureAtlasIndicesGetFrames(TextureAtlasIndices *indices, TextureAtlasFrames *frames, String *name)
{
    int index = TextureAtlasIndicesGetIndex(indices, name);
    if (index == -1)
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

int main(void)
{
    Arena *arena = ArenaAlloc(128 * Megabyte);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Pixel Art Hint
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Capy Quest", 100, 100, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    // Window scale factor
    int windowScaleFactor = 4;

    // Get the window size
    int windowRealWidth = 0;
    int windowRealHeight = 0;
    SDL_GetWindowSize(window, &windowRealWidth, &windowRealHeight);

    // Get the scaled window size
    int windowWidth = windowRealWidth / windowScaleFactor;
    int windowHeight = windowRealHeight / windowScaleFactor;

    // Set the logical size of the renderer
    SDL_RenderSetLogicalSize(renderer, windowWidth, windowHeight);

    SDL_Texture *atlasTexture;
    u16 atlasWidth = 0;
    u16 atlasHeight = 0;

    ARRAY_ALLOC(arena, TextureAtlasIndex, textureAtlasIndices, 128);
    ARRAY_ALLOC(arena, SDL_Rect, textureAtlasFrames, 128);

    {
        Arena *textureAtlasArena = ArenaAlloc(128 * Megabyte);
        tempMemoryBlock(textureAtlasArena)
        {
            // Path to glob for sprites
            String spriteAssetsGlob = STR("../assets/sprites/*.aseprite");

            // Sprite assets memory
            typedef struct SpriteAssetPath
            {
                String name;
                String path;
            } SpriteAssetPath;
            ARRAY(SpriteAssetPath, spriteAssetPaths);

            {
                // Glob all the sprite assets
                glob_t globResult;
                glob(spriteAssetsGlob.ptr, GLOB_TILDE, NULL, &globResult);

                // Allocate space for the paths
                spriteAssetPaths.len = globResult.gl_pathc;
                spriteAssetPaths.ptr = ArenaPushArrayZero(textureAtlasArena, spriteAssetPaths.len, SpriteAssetPath);

                printf("Found %zu sprite assets\n", spriteAssetPaths.len);

                // Get the paths;
                for (usize i = 0; i < spriteAssetPaths.len; i++)
                {
                    // Copy each path from the glob
                    String *pathString = StringCopyCString(textureAtlasArena, globResult.gl_pathv[i]);
                    spriteAssetPaths.ptr[i].path = *pathString;

                    // Get the name of the sprite asset without the extension and path
                    String *nameString = StringCopy(textureAtlasArena, pathString);
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
            ARRAY_ALLOC_RESERVED(textureAtlasArena, AsepriteFile, sprites, spriteAssetPaths.len);

            // Allocate space for the sprite frames
            ARRAY_ALLOC(textureAtlasArena, AsepriteAnimationFrame, spriteFrames, 128);

            for (usize i = 0; i < sprites.len; i++)
            {
                // Read the sprite asset file
                String *spriteAssetPath = &spriteAssetPaths.ptr[i].path;
                String *spriteAssetName = &spriteAssetPaths.ptr[i].name;

                printf("Loading sprite asset: %s = %s\n", spriteAssetName->ptr, spriteAssetPath->ptr);
                AsepriteFile *sprite = &sprites.ptr[i];
                sprite = AsepriteLoad(textureAtlasArena, spriteAssetPath);

                // Add the sprite asset to the atlas index
                TextureAtlasIndex atlasIndex = {
                    .numFrames = sprite->numFrames,
                    .frameIndex = spriteFrames.len,
                    .name = StringCopy(textureAtlasArena, spriteAssetName)};
                ARRAY_PUSH(arena, textureAtlasIndices, TextureAtlasIndex, atlasIndex);

                // Get all the the frames
                for (usize j = 0; j < sprite->numFrames; j++)
                {
                    AsepriteAnimationFrame spriteFrameProcessed;
                    AsepriteGetAnimationFrame(sprite, j, &spriteFrameProcessed);
                    ARRAY_PUSH(textureAtlasArena, spriteFrames, AsepriteAnimationFrame, spriteFrameProcessed);
                }
            }

            ARRAY_ALLOC_RESERVED(textureAtlasArena, stbrp_rect, spriteRects, spriteFrames.len);
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
                stbrp_node *nodes = ArenaPushArray(textureAtlasArena, spriteFrames.len, stbrp_node);
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
                if (rect->x + rect->w > atlasWidth)
                {
                    atlasWidth = rect->x + rect->w;
                }

                if (rect->y + rect->h > atlasHeight)
                {
                    atlasHeight = rect->y + rect->h;
                }
            }

            // Allocate space for the atlas
            u32 *atlasPixels = ArenaPushArrayZero(textureAtlasArena, atlasWidth * atlasHeight, u32);

            // Copy the sprite frames into the atlas
            for (usize i = 0; i < spriteFrames.len; i++)
            {
                // Get the rect for the sprite frame
                stbrp_rect *rect = &spriteRects.ptr[i];

                // Push the rect to the atlas rects
                {
                    SDL_Rect atlasRect = {rect->x, rect->y, rect->w, rect->h};
                    ARRAY_PUSH(arena, textureAtlasFrames, SDL_Rect, atlasRect);
                }

                // Get the frame pixels and copy them into the atlas
                AsepriteAnimationFrame *spriteFrame = &spriteFrames.ptr[rect->id];
                for (u16 y = 0; y < spriteFrame->sizeY; y++)
                {
                    for (u16 x = 0; x < spriteFrame->sizeX; x++)
                    {
                        u32 *atlasPixel = &atlasPixels[(rect->y + y) * atlasWidth + (rect->x + x)];
                        u32 *spritePixel = &spriteFrame->pixels[y * spriteFrame->sizeX + x];

                        *atlasPixel = *spritePixel;
                    }
                }
            }

            // Create a texture from the atlas
            atlasTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, atlasWidth, atlasHeight);

            // Copy the atlas pixels into the texture
            SDL_UpdateTexture(atlasTexture, NULL, atlasPixels, atlasWidth * sizeof(u32));

            // Allow for alpha blending
            SDL_SetTextureBlendMode(atlasTexture, SDL_BLENDMODE_BLEND);
        }
    }

    // Get the capy sprite frames
    TextureAtlasFrames capyFrames = TextureAtlasIndicesGetFrames(&textureAtlasIndices, &textureAtlasFrames, &STR("capy_indle"));

    // Get the first frame of the capy sprite
    SDL_Rect capyTextureRect = capyFrames.ptr[0];

    u64 time = 0;

    SDL_Event event;
    bool running = true;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        if (time % 20 == 0)
        {
            capyTextureRect = capyFrames.ptr[time / 20 % capyFrames.len];
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 128, 200, 255);
        SDL_RenderClear(renderer);

        // Draw a rect from the atlas texture
        SDL_Rect drawRect = {0, 0, capyTextureRect.w, capyTextureRect.h};
        SDL_RenderCopy(renderer, atlasTexture, &capyTextureRect, &drawRect);

        // Update the screen
        SDL_RenderPresent(renderer);

        // 60 FPS
        SDL_Delay(16);

        time++;
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    ArenaFree(arena);

    return 0;
}
