cmake_minimum_required(VERSION 3.18)

project(capy-quest)

set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE SOURCES src/*.c)
include_directories(src/)

# Set debug symbols flag
set(CMAKE_BUILD_TYPE Debug)

# Set compiler flags
set(CMAKE_C_FLAGS "-fsanitize=address -Wall -Wextra -pedantic -g -O0")

# Find packages
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

# Include packages
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})

# Include external
include_directories(external/)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} SDL2::SDL2)