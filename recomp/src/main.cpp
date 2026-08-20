#include <iostream>
#include <cstdint>
#include <vector>
#include <string>

#define SDL_MAIN_HANDLED
#include <SDL.h>

// Representación del espacio de memoria virtual de la N64 (8 MB RDRAM)
uint8_t rdram[8 * 1024 * 1024] = { 0 };

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::cout << "========================================" << std::endl;
    std::cout << " Conker's Bad Fur Day: Recompiled (PC)" << std::endl;
    std::cout << " Version: Native Windows x64 Build" << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "[Init] Allocating 8MB N64 RDRAM space... OK" << std::endl;

    // 1. Inicializar SDL2 para Video, Audio y Mandos
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "[Error] Failed to initialize SDL2: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "[Init] SDL2 Initialized... OK" << std::endl;

    // 2. Crear la Ventana Nativa de PC
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

    // 3. Crear Renderizador Básico con V-Sync
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "[Error] Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "[Window] 1280x720 Native Window Created Successfully!" << std::endl;
    std::cout << "[Loop] Running main application event loop... (Close window or press ESC to exit)" << std::endl;

    // Variables para cálculo en tiempo real de FPS
    uint32_t frameCount = 0;
    uint32_t lastFpsUpdate = SDL_GetTicks();
    float currentFps = 0.0f;

    // 4. Bucle Principal de Eventos (Event Loop)
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

        // Medidor de FPS en tiempo real
        frameCount++;
        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - lastFpsUpdate >= 500) { // Actualizar cada medio segundo
            currentFps = (frameCount * 1000.0f) / (currentTicks - lastFpsUpdate);
            std::string title = "Conker's Bad Fur Day: Recompiled (PC Native) | FPS: " + 
                                std::to_string(static_cast<int>(currentFps)) + 
                                " | 60Hz V-Sync Active";
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsUpdate = currentTicks;
        }

        // Limpiar pantalla con color oscuro N64 (Dark Slate Gray)
        SDL_SetRenderDrawColor(renderer, 20, 24, 30, 255);
        SDL_RenderClear(renderer);

        // Presentar cuadro
        SDL_RenderPresent(renderer);
    }

    // 5. Limpieza al salir
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "[Exit] Conker Recompiled closed cleanly." << std::endl;
    return 0;
}
