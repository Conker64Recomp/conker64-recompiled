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
#include "audio.hpp"
#include "mips_recomp.hpp"
#include "texture_loader.hpp"

// Diálogo de explorador de archivos nativo de Windows
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
    std::cout << " Engine: SDL2 + Textures + 3D UV Engine" << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. AppData Paths
    std::cout << "[Paths] Saves: " << N64::PathManager::getAppDataPath() << std::endl;
    std::cout << "[Paths] Cache: " << N64::PathManager::getCachePath() << std::endl;

    // 2. Hardware Cores
    N64::Memory::getInstance().init();
    N64::SaveSystem::getInstance().init();
    N64::MIPSRecompiler::getInstance().init();

    // 3. Inicializar SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "[Error] Failed to initialize SDL2: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    std::cout << "[Init] SDL2 Video & Audio Systems Initialized... OK" << std::endl;

    // 4. Inicializar Audio
    N64::AudioManager::getInstance().init(44100);

    // 5. Input Manager
    N64::InputManager::getInstance().init();

    // 6. Crear Ventana Nativa
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
        N64::AudioManager::getInstance().shutdown();
        SDL_Quit();
        return 1;
    }

    // 7. Renderizador Acelerado por GPU
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "[Error] Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        N64::AudioManager::getInstance().shutdown();
        SDL_Quit();
        return 1;
    }

    // 8. Inicializar RDP con soporte de textura
    N64::RDPProcessor::getInstance().init(renderer);

    // 9. Carga de la ROM
    bool romLoaded = false;
    std::vector<std::string> searchPaths = {
        "baserom.us.z64",
        "../baserom.us.z64",
        "../../baserom.us.z64",
        "Conker's Bad Fur Day (USA).z64"
    };

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            if (N64::ROMLoader::loadROM(path)) {
                romLoaded = true;
                N64::MIPSRecompiler::getInstance().executeBootFunction();
                break;
            }
        }
    }

    uint32_t frameCount = 0;
    uint32_t lastFpsUpdate = SDL_GetTicks();
    float currentFps = 0.0f;
    float rotationAngle = 0.0f;

    N64::OSContPad pad{};

    // 10. Bucle Principal
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            else if (event.type == SDL_DROPFILE) {
                char* droppedFile = event.drop.file;
                std::cout << "[Launcher] ROM Arrastrada: " << droppedFile << std::endl;
                if (N64::ROMLoader::loadROM(droppedFile)) {
                    romLoaded = true;
                    N64::MIPSRecompiler::getInstance().executeBootFunction();
                }
                SDL_free(droppedFile);
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                else if (event.key.keysym.sym == SDLK_o) {
                    std::string selected = openFileDialog();
                    if (!selected.empty()) {
                        if (N64::ROMLoader::loadROM(selected)) {
                            romLoaded = true;
                            N64::MIPSRecompiler::getInstance().executeBootFunction();
                        }
                    }
                }
                else if (event.key.keysym.sym == SDLK_t) {
                    N64::AudioManager::getInstance().playBootJingle();
                }
            }
        }

        // Mandos / Teclado
        N64::InputManager::getInstance().poll(pad);

        // Control 3D
        float rotSpeed = 1.2f;
        if (std::abs(pad.stick_x) > 10) {
            rotSpeed = (pad.stick_x / 80.0f) * 4.0f;
        }
        rotationAngle += rotSpeed;
        if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
        if (rotationAngle < 0.0f) rotationAngle += 360.0f;

        // Medidor de FPS y Estado en Título
        frameCount++;
        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - lastFpsUpdate >= 500) {
            currentFps = (frameCount * 1000.0f) / (currentTicks - lastFpsUpdate);
            std::string title = "";
            if (romLoaded) {
                title = "Conker64: Recompiled | 3D Textures & UV Engine Active | FPS: " + 
                        std::to_string(static_cast<int>(currentFps)) + " | 60Hz";
            } else {
                title = "Conker64: Recompiled | ESPERANDO ROM (Arrastra tu .z64 o presiona O)";
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

        // Renderizado 3D con Texturas Mapeadas
        N64::RDPProcessor::getInstance().processDisplayList(0, renderer, winW, winH, rotationAngle);

        SDL_RenderPresent(renderer);
    }

    // Limpieza
    N64::RDPProcessor::getInstance().shutdown();
    N64::SaveSystem::getInstance().saveEEPROM();
    N64::AudioManager::getInstance().shutdown();
    N64::InputManager::getInstance().shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "[Exit] Conker Recompiled closed cleanly." << std::endl;
    return 0;
}
