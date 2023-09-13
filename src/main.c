#define GL_SILENCE_DEPRECATION 1
#include <GLFW/glfw3.h>

#include "arena.h"
#include "aseprite.h"
#include "fs.h"
#include "str.h"
#include "util.h"

int main(void)
{
    Arena *arena = ArenaAlloc(512 * Megabyte);

    tempMemoryBlock(arena)
    {
        // NOTE(SeedyROM): this is just an excuse to test the StringBuilder
        StringBuilder *sb = StringBuilderAlloc(arena);
        StringBuilderAppend(sb, STR("../assets/sprites/"));
        StringBuilderAppend(sb, STR("capy1.aseprite"));
        String path = sb->string;

        // Load the aesprite file and print some info
        AsepriteFile *file = AsepriteLoad(arena, &path);
        for (u16 frameIndex = 0; frameIndex < file->numFrames; frameIndex++)
        {
            AsepriteFrame *frame = AsepriteGetFrame(arena, file, frameIndex);
            printf("Frame %d: %dx%d\n", frameIndex, frame->sizeX, frame->sizeY);
        }
    }

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
