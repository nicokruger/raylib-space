cmake_minimum_required(VERSION 3.15)
project(my_project)
set(CMAKE_BUILD_TYPE Debug)

find_package(raylib 3.0 REQUIRED) # Requires at least version 3.0

set(CMAKE_C_STANDARD 11) # Requires C11 standard

add_executable(${PROJECT_NAME} main.cpp ecs.cpp vendor/flecs/flecs.c)

target_link_libraries(${PROJECT_NAME} raylib m)
target_include_directories(${PROJECT_NAME} PRIVATE vendor/flecs)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
