#include <stdio.h>
#include <assert.h>

#define GL_SILENCE_DEPRECATION 1
#include <GLFW/glfw3.h>

#include "arena.h"
#include "fs.h"
#include "str.h"
#include "util.h"

int main(void)
{
    Arena *arena = ArenaAlloc(1 * Gigabyte);

    Arena *scratch = ArenaAlloc(16 * Megabyte);
    tempMemoryBlock(scratch)
    {
        // NOTE(SeedyROM): this is just an excuse to test the StringBuilder
        StringBuilder *sb = StringBuilderAlloc(scratch);
        StringBuilderAppend(sb, STR("../assets/sprites/"));
        StringBuilderAppend(sb, STR("capy1.aseprite"));
        String path = sb->string;

        struct sprite
        {
            usize offset;
            ByteArray *data;
        } sprite;
        sprite.offset = 0;
        sprite.data = ReadFileBytes(arena, &path);
        printf("File size: %zu\n", sprite.data->len);

        // NOTE(SeedyROM): Should this be checked?
        u32 spriteSize = ByteArrayReadU32(sprite.data, &sprite.offset);
        assert(spriteSize == sprite.data->len);

        // Check the magic number
        u16 magic = ByteArrayReadU16(sprite.data, &sprite.offset);
        assert(magic == 0xA5E0);

        // Check the number of frames
        u16 numFrames = ByteArrayReadU16(sprite.data, &sprite.offset);
        printf("Number of frames: %u\n", numFrames);

        // Check the width and height
        u16 width = ByteArrayReadU16(sprite.data, &sprite.offset);
        u16 height = ByteArrayReadU16(sprite.data, &sprite.offset);
        printf("Width: %u\n", width);
        printf("Height: %u\n", height);
    };
    ArenaFree(scratch);

    GLFWwindow *window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Close if q or escape is pressed
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    ArenaFree(arena);
    return 0;
}
