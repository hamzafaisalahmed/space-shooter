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
set(FONT_DST "${CMAKE_BINARY_DIR}/assets/game_font.ttf")

if(NOT EXISTS "${FONT_DST}")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/assets")

    if(EXISTS "C:/Windows/Fonts/arial.ttf")
        file(COPY "C:/Windows/Fonts/arial.ttf" DESTINATION "${CMAKE_BINARY_DIR}/assets")
        file(RENAME "${CMAKE_BINARY_DIR}/assets/arial.ttf" "${FONT_DST}")
        message(STATUS "Font: copied Arial from C:/Windows/Fonts")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/assets/game_font.ttf")
        file(COPY "${CMAKE_SOURCE_DIR}/assets/game_font.ttf"
             DESTINATION "${CMAKE_BINARY_DIR}/assets")
        message(STATUS "Font: copied game_font.ttf from project assets/")
    else()
        message(WARNING "No font found! Place any .ttf as SpaceShooter2/assets/game_font.ttf then reconfigure.")
    endif()
endif()