cmake_minimum_required(VERSION 3.21)
project(SpaceShooter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# SFML — use the pacman-installed copy (ucrt64 toolchain).
# Install first if you haven't:
#   pacman -S mingw-w64-ucrt-x86_64-sfml
# ============================================================
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# ============================================================
# GAME EXECUTABLE
# ============================================================
add_executable(SpaceShooter src/main.cpp)

target_link_libraries(SpaceShooter
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        opengl32
        winmm
        gdi32
)

target_compile_options(SpaceShooter PRIVATE -Wall)

# Release: hide the console window
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(SpaceShooter PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_options(SpaceShooter PRIVATE -mwindows)
endif()

# ============================================================
# FONT
# ============================================================

# 1. This ensures the 'assets' folder is copied every time you build
add_custom_command(TARGET SpaceShooter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:SpaceShooter>/assets"
    COMMENT "Updating assets in build directory..."
)
