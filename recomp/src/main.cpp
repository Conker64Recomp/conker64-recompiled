#include <iostream>
#include <cstdint>
#include <vector>
#include <string>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "paths.hpp"
#include "memory.hpp"
#include "save_system.hpp"
#include "vi.hpp"
#include "input.hpp"
#include "rom_loader.hpp"
#include "rdp.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::cout << "========================================" << std::endl;
    std::cout << " Conker's Bad Fur Day: Recompiled (PC)" << std::endl;
    std::cout << " Version: Native Windows x64 Build" << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Mostrar ubicaciones seguras de AppData y Cache
    std::cout << "[Paths] Saves: " << N64::PathManager::getAppDataPath() << std::endl;
    std::cout << "[Paths] Cache: " << N64::PathManager::getCachePath() << std::endl;

    // 2. Memoria RDRAM
    N64::Memory::getInstance().init();

    // 3. Sistema de Guardado Nativo
    N64::SaveSystem::getInstance().init();

    // 4. Cargar ROM original de Nintendo 64
    std::string romPath = "../baserom.us.z64";
    if (!N64::ROMLoader::loadROM(romPath)) {
        romPath = "../../baserom.us.z64";
        N64::ROMLoader::loadROM(romPath);
    }

    // 5. RDP / Fast3D Display List Processor
    N64::RDPProcessor::getInstance().init();

    // 6. SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "[Error] Failed to initialize SDL2: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "[Init] SDL2 Initialized... OK" << std::endl;

    // 7. Input Manager
    N64::InputManager::getInstance().init();

    // 8. Crear Ventana Nativa
    int windowWidth = 1280;
    int windowHeight = 720;

    SDL_Window* window = SDL_CreateWindow(
        "Conker's Bad Fur Day: Recompiled (PC Native) - Initializing...",
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

    // 9. Renderizador Acelerado por GPU con V-Sync
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "[Error] Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "[Window] 1280x720 Native Hardware Accelerated Window Ready!" << std::endl;
    std::cout << "[Loop] Running 3D Graphics pipeline at 60 FPS... (Press ESC to exit)" << std::endl;

    uint32_t frameCount = 0;
    uint32_t lastFpsUpdate = SDL_GetTicks();
    float currentFps = 0.0f;
    float rotationAngle = 0.0f;

    N64::OSContPad pad{};

    // 10. Bucle Principal de Renderizado 3D
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
            }
        }

        // Leer mando / teclado
        N64::InputManager::getInstance().poll(pad);

        // Controlar velocidad de giro 3D con el stick analógico o botones
        float rotSpeed = 1.2f;
        if (std::abs(pad.stick_x) > 10) {
            rotSpeed = (pad.stick_x / 80.0f) * 4.0f;
        }
        rotationAngle += rotSpeed;
        if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
        if (rotationAngle < 0.0f) rotationAngle += 360.0f;

        // Medidor de FPS en tiempo real
        frameCount++;
        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - lastFpsUpdate >= 500) {
            currentFps = (frameCount * 1000.0f) / (currentTicks - lastFpsUpdate);
            std::string title = "Conker's Bad Fur Day: Recompiled (PC) | FPS: " + 
                                std::to_string(static_cast<int>(currentFps)) + 
                                " | RDP Fast3D Pipeline Active [60Hz]";
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsUpdate = currentTicks;
        }

        // Limpiar pantalla con color oscuro N64
        SDL_SetRenderDrawColor(renderer, 15, 18, 24, 255);
        SDL_RenderClear(renderer);

        // Obtener tamaño actual de la ventana
        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);

        // Procesar y Renderizar gráficos 3D mediante el microcódigo RDP
        N64::RDPProcessor::getInstance().processDisplayList(0, renderer, winW, winH, rotationAngle);

        // Presentar cuadro a la pantalla
        SDL_RenderPresent(renderer);
    }

    // Guardar estado de la EEPROM al salir
    N64::SaveSystem::getInstance().saveEEPROM();

    // Limpieza
    N64::InputManager::getInstance().shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "[Exit] Conker Recompiled closed cleanly." << std::endl;
    return 0;
}
