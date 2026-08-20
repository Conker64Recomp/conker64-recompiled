cmake_minimum_required(VERSION 3.20)
project(ConkerRecompiled LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include(FetchContent)

# Descargar e integrar SDL3 / SDL2 automáticamente vía CMake
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

# Ejecutable principal
add_executable(Conker
    src/main.cpp
    src/ultralib_stubs.cpp
)

# Inclusiones y Enlaces
target_include_directories(Conker PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Conker PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
)

# Copiar SDL2.dll a la carpeta de salida automáticamente
add_custom_command(TARGET Conker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:Conker>
)

if(MSVC)
    target_compile_options(Conker PRIVATE /W4 /utf-8 /permissive-)
endif()
