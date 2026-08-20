cmake_minimum_required(VERSION 3.20)
project(ConkerRecompiled LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include(FetchContent)

# Descargar e integrar SDL2
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.2
    GIT_SHALLOW TRUE
)

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL2)

# Descargar e integrar zlib (para descomprimir assets Rareware / deflate raw)
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
    GIT_SHALLOW TRUE
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zlib)

# Ejecutable principal
add_executable(Conker
    src/main.cpp
    src/ultralib_stubs.cpp
)

# Inclusiones explícitas para IntelliSense y compilación
target_include_directories(Conker PUBLIC
    include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${sdl2_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl2-build/include
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl2-build/include-config-release/SDL2
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl2-build/include-config-debug/SDL2
    ${zlib_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/zlib-build
)

target_link_libraries(Conker PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    zlibstatic
)

# Copiar SDL2.dll a la carpeta del ejecutable
add_custom_command(TARGET Conker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:Conker>
)

if(MSVC)
    target_compile_options(Conker PRIVATE /W4 /utf-8 /permissive- /wd4100 /wd4244 /wd4996)
endif()
