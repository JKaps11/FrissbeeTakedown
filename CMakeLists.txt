cmake_minimum_required(VERSION 3.10)
project(MyRaylibProject C) # Specify C language

# Find raylib package (assumes raylib is installed in system paths or a package manager path)
find_package(raylib REQUIRED)

# Add your executable
add_executable(${PROJECT_NAME} main.c camera.c map.c player.c frisbee.c game.c enemy.c)

# Link the raylib library to your executable
target_link_libraries(${PROJECT_NAME} PRIVATE raylib m)

# Copy media files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/media DESTINATION ${CMAKE_BINARY_DIR})

