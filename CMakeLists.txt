cmake_minimum_required(VERSION 3.18)

project(capy-quest)

set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE SOURCES src/*.c)
include_directories(src/)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set compiler flags
set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-fsanitize=address -g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O3")

# Find packages
find_package(SDL2 REQUIRED)

# Include packages
include_directories(${SDL2_INCLUDE_DIRS})

# Include external
include_directories(external/)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2-static m)
