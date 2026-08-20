cmake_minimum_required(VERSION 3.20)
project(ConkerRecompiled LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Configuración del ejecutable principal
add_executable(Conker
    src/main.cpp
    src/ultralib_stubs.cpp
)

# Inclusiones
target_include_directories(Conker PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Definiciones para Windows / MSVC
if(MSVC)
    target_compile_options(Conker PRIVATE /W4 /utf-8 /permissive-)
endif()
