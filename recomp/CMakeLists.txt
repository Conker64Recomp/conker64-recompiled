cmake_minimum_required(VERSION 3.20)
project(ConkerRecompiled LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Runtime de MSVC enlazado estaticamente: el binario distribuido arranca sin
# exigir el redistribuible de Visual C++. Debe fijarse ANTES de FetchContent
# para que SDL2 y zlib usen el mismo CRT; mezclarlos rompe el enlazado.
option(CONKER_STATIC_CRT "Link the MSVC runtime statically (self-contained binary)" ON)
if (MSVC AND CONKER_STATIC_CRT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
endif()

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

# Rutas a N64ModernRuntime y RT64
set(RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../tools/N64ModernRuntime")
set(RUNTIME_BUILD_DIR "${RUNTIME_DIR}/build")
set(RT64_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../tools/rt64")
set(RT64_BUILD_DIR "${RT64_DIR}/build_nmake")

# Ejecutable principal
add_executable(Conker
    src/main.cpp
    src/ultralib_stubs.cpp
)

# <windows.h> define macros min/max que rompen std::min/std::max y
# std::numeric_limits<T>::max(). Hay que silenciarlas antes de cualquier
# inclusion; hacerlo en un header suelto llega tarde.
if (WIN32)
    target_compile_definitions(Conker PRIVATE NOMINMAX)
endif()

# Inclusiones
target_include_directories(Conker PUBLIC
    include
    src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${sdl2_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl2-build/include
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl2-build/include-config-release/SDL2
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl2-build/include-config-debug/SDL2
    ${zlib_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/_deps/zlib-build
    ${RUNTIME_DIR}/ultramodern/include
    ${RUNTIME_DIR}/librecomp/include
    ${RUNTIME_DIR}/N64Recomp/include
    ${RUNTIME_DIR}/thirdparty
    ${RT64_DIR}/src
    ${RT64_DIR}/src/contrib/json
)

# Librerías estáticas y dinámicas
find_library(LIB_ULTRAMODERN ultramodern PATHS "${RUNTIME_BUILD_DIR}/ultramodern/Release" NO_DEFAULT_PATH)
find_library(LIB_LIBRECOMP librecomp PATHS "${RUNTIME_BUILD_DIR}/librecomp/Release" NO_DEFAULT_PATH)
find_library(LIB_MINIZ miniz PATHS "${RUNTIME_BUILD_DIR}/librecomp/miniz/Release" NO_DEFAULT_PATH)
find_library(LIB_RT64 rt64 PATHS "${RT64_BUILD_DIR}" NO_DEFAULT_PATH)

target_link_libraries(Conker PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    zlibstatic
)

if (LIB_ULTRAMODERN)
    target_link_libraries(Conker PRIVATE ${LIB_ULTRAMODERN})
    message(STATUS "[N64ModernRuntime] Linked ultramodern.lib")
endif()

if (LIB_LIBRECOMP)
    target_link_libraries(Conker PRIVATE ${LIB_LIBRECOMP})
    message(STATUS "[N64ModernRuntime] Linked librecomp.lib")
endif()

if (LIB_MINIZ)
    target_link_libraries(Conker PRIVATE ${LIB_MINIZ})
    message(STATUS "[N64ModernRuntime] Linked miniz.lib")
endif()

if (LIB_RT64)
    target_link_libraries(Conker PRIVATE ${LIB_RT64})
    message(STATUS "[RT64] Linked rt64.lib (DirectX 12 / Ray Tracing)")
endif()

# Copiar SDL2.dll y rt64.dll a la carpeta del ejecutable
add_custom_command(TARGET Conker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:Conker>
)

if (EXISTS "${RT64_BUILD_DIR}/rt64.dll")
    add_custom_command(TARGET Conker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RT64_BUILD_DIR}/rt64.dll"
        $<TARGET_FILE_DIR:Conker>
    )
endif()
