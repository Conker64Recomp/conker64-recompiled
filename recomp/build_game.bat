@echo off
set "VC_DIR=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.29.30133"
set "SDK_DIR=C:\Program Files (x86)\Windows Kits\10"
set "SDK_VER=10.0.26100.0"

set "PATH=%VC_DIR%\bin\HostX64\x64;%SDK_DIR%\bin\%SDK_VER%\x64;C:\Program Files\CMake\bin;%PATH%"
set "INCLUDE=%VC_DIR%\include;%SDK_DIR%\Include\%SDK_VER%\ucrt;%SDK_DIR%\Include\%SDK_VER%\um;%SDK_DIR%\Include\%SDK_VER%\shared"
set "LIB=%VC_DIR%\lib\x64;%SDK_DIR%\Lib\%SDK_VER%\ucrt\x64;%SDK_DIR%\Lib\%SDK_VER%\um\x64"

echo [Build] Configurando entorno MSVC x64 con Windows SDK %SDK_VER%...

if exist build rd /s /q build

echo [Build] Configurando proyecto con CMake y NMake...
cmake -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release

echo [Build] Compilando Conker Recompiled...
cmake --build build

echo [Build] Proceso finalizado.
