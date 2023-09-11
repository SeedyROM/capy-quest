cmake_minimum_required(VERSION 3.18)

project(capy-quest)

set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE SOURCES src/*.c)
include_directories(src/)

# Set debug symbols flag
set(CMAKE_BUILD_TYPE Debug)

# Set compiler flags
set(CMAKE_C_FLAGS "-fsanitize=address -Wall  -Wextra -pedantic -g -O0")

find_package(GLFW3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL)