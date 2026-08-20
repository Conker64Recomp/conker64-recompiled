#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

#include "paths.hpp"
#include "memory.hpp"
#include "save_system.hpp"
#include "vi.hpp"
#include "input.hpp"
#include "rom_loader.hpp"
#include "rdp.hpp"

// Abre la ventana nativa de explorador de archivos de Windows para seleccionar la ROM
std::string openFileDialog() {
#ifdef _WIN32
    char filename[MAX_PATH] = { 0 };
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Nintendo 64 ROMs (*.z64;*.n64;*.v64)\0*.z64;*.n64;*.v64\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle = "Selecciona tu ROM de Conker's Bad Fur Day (USA)";

    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
#endif
    return "";
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::cout << "========================================" << std::endl;
    std::cout << " CONKER64: RECOMPILED (NATIVE PC PORT)" << std::endl;
    std::cout << " Clean Launcher & Hardware 3D Engine" << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Mostrar ubicaciones seguras de AppData y Cache
    std::cout << "[Paths] Saves: " << N64::PathManager::getAppDataPath() << std::endl;
    std::cout << "[Paths] Cache: " << N64::PathManager::getCachePath() << std::endl;

    // 2. Memoria RDRAM & Save System
    N64::Memory::getInstance().init();
    N64::SaveSystem::getInstance().init();
    N64::RDPProcessor::getInstance().init();

    // 3. Inicializar SDL2 con soporte para Drag & Drop de archivos
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "[Error] Failed to initialize SDL2: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    std::cout << "[Init] SDL2 Initialized with Drag & Drop support... OK" << std::endl;

    // 4. Input Manager
    N64::InputManager::getInstance().init();

    // 5. Crear Ventana Nativa de PC
    int windowWidth = 1280;
    int windowHeight = 720;

    SDL_Window* window = SDL_CreateWindow(
        "Conker64: Recompiled - Arrastra tu ROM (.z64) o Presiona [O] para Buscar",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) {
        std::cerr << "[Error] Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 6. Renderizador Acelerado por GPU
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "[Error] Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 7. Auto-detectar si ya existe la ROM en la carpeta o iniciar en modo espera
    bool romLoaded = false;
    std::string loadedRomName = "";

    std::vector<std::string> searchPaths = {
        "baserom.us.z64",
        "../baserom.us.z64",
        "../../baserom.us.z64",
        "Conker's Bad Fur Day (USA).z64",
        "../Conker's Bad Fur Day (USA).z64"
    };

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            if (N64::ROMLoader::loadROM(path)) {
                romLoaded = true;
                loadedRomName = path;
                break;
            }
        }
    }

    uint32_t frameCount = 0;
    uint32_t lastFpsUpdate = SDL_GetTicks();
    float currentFps = 0.0f;
    float rotationAngle = 0.0f;

    N64::OSContPad pad{};

    // 8. Bucle Principal de la Aplicación
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            // Evento: Arrastrar y soltar archivo ROM sobre la ventana (Drag & Drop)
            else if (event.type == SDL_DROPFILE) {
                char* droppedFile = event.drop.file;
                std::cout << "[Launcher] ROM Arrastrada a la ventana: " << droppedFile << std::endl;
                if (N64::ROMLoader::loadROM(droppedFile)) {
                    romLoaded = true;
                    loadedRomName = droppedFile;
                }
                SDL_free(droppedFile);
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                // Presionar tecla 'O' para abrir explorador de archivos y elegir ROM
                else if (event.key.keysym.sym == SDLK_o) {
                    std::string selected = openFileDialog();
                    if (!selected.empty()) {
                        std::cout << "[Launcher] ROM Seleccionada: " << selected << std::endl;
                        if (N64::ROMLoader::loadROM(selected)) {
                            romLoaded = true;
                            loadedRomName = selected;
                        }
                    }
                }
            }
        }

        // Leer mandos / teclado
        N64::InputManager::getInstance().poll(pad);

        // Controlar velocidad de giro 3D con el stick analógico o botones
        float rotSpeed = 1.2f;
        if (std::abs(pad.stick_x) > 10) {
            rotSpeed = (pad.stick_x / 80.0f) * 4.0f;
        }
        rotationAngle += rotSpeed;
        if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
        if (rotationAngle < 0.0f) rotationAngle += 360.0f;

        // Medidor de FPS en tiempo real y estado del Launcher en el título
        frameCount++;
        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - lastFpsUpdate >= 500) {
            currentFps = (frameCount * 1000.0f) / (currentTicks - lastFpsUpdate);
            std::string title = "";
            if (romLoaded) {
                title = "Conker64: Recompiled | ROM: CONKER BFD (USA) [VERIFICADA] | FPS: " + 
                        std::to_string(static_cast<int>(currentFps)) + " | 3D Engine Activo";
            } else {
                title = "Conker64: Recompiled | ESPERANDO ROM (Arrastra tu .z64 a la ventana o presiona O)";
            }
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsUpdate = currentTicks;
        }

        // Limpiar pantalla
        SDL_SetRenderDrawColor(renderer, 15, 18, 24, 255);
        SDL_RenderClear(renderer);

        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);

        // Renderizado 3D en tiempo real
        N64::RDPProcessor::getInstance().processDisplayList(0, renderer, winW, winH, rotationAngle);

        // Presentar cuadro
        SDL_RenderPresent(renderer);
    }

    // Guardar partida al salir
    N64::SaveSystem::getInstance().saveEEPROM();

    // Limpieza
    N64::InputManager::getInstance().shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "[Exit] Conker Recompiled closed cleanly." << std::endl;
    return 0;
}
