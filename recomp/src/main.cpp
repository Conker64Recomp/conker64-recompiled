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

// Dibuja el patrón dinámico + la mira interactiva controlada con mando o WASD
void drawInteractiveFrame(uint32_t fbVaddr, int width, int height, uint32_t tick, int cursorX, int cursorY, bool buttonA, bool buttonB) {
    uint8_t* rdram = N64::Memory::getInstance().getRDRAM();
    uint32_t paddr = fbVaddr & 0x1FFFFFFF;
    uint16_t* fb = reinterpret_cast<uint16_t*>(&rdram[paddr]);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t r = ((x + tick / 20) & 0x1F);
            uint8_t g = ((y + tick / 20) & 0x1F);
            uint8_t b = (((x + y) / 2) & 0x1F);

            if (buttonA) r = 0x1F;
            if (buttonB) b = 0x1F;

            uint16_t color = (r << 11) | (g << 6) | (b << 1) | 1;
            fb[y * width + x] = ((color >> 8) & 0xFF) | ((color << 8) & 0xFF00);
        }
    }

    // Mira interactiva
    int cx = width / 2 + cursorX;
    int cy = height / 2 - cursorY;

    for (int dy = -6; dy <= 6; ++dy) {
        for (int dx = -6; dx <= 6; ++dx) {
            int px = cx + dx;
            int py = cy + dy;
            if (px >= 0 && px < width && py >= 0 && py < height) {
                if (dx == 0 || dy == 0 || (std::abs(dx) == std::abs(dy))) {
                    uint16_t crossColor = 0xFFFF;
                    fb[py * width + px] = ((crossColor >> 8) & 0xFF) | ((crossColor << 8) & 0xFF00);
                }
            }
        }
    }
}

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

    // 3. Sistema de Guardado Nativo (EEPROM 16Kbit persistente del juego)
    N64::SaveSystem::getInstance().init();

    // 4. Cargar ROM original de Nintendo 64
    std::string romPath = "../baserom.us.z64";
    if (!N64::ROMLoader::loadROM(romPath)) {
        romPath = "../../baserom.us.z64";
        N64::ROMLoader::loadROM(romPath);
    }

    // 5. SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "[Error] Failed to initialize SDL2: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "[Init] SDL2 Initialized... OK" << std::endl;

    // 6. Input Manager
    N64::InputManager::getInstance().init();

    // 7. Crear Ventana Nativa
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

    // 8. Renderizador con V-Sync
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "[Error] Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 9. Video Interface (VI)
    uint32_t fbVaddr = 0x80100000;
    N64::VideoInterface vi;
    vi.setWidth(320);
    vi.setHeight(240);
    vi.setOrigin(fbVaddr);
    vi.setFormat(2);

    SDL_Texture* viTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        vi.getWidth(),
        vi.getHeight()
    );

    std::cout << "[VI] N64 Video Interface setup at " << vi.getWidth() << "x" << vi.getHeight() << " (RGBA5551)" << std::endl;
    std::cout << "[Loop] Running main application event loop... (Close window or press ESC to exit)" << std::endl;

    uint32_t frameCount = 0;
    uint32_t lastFpsUpdate = SDL_GetTicks();
    float currentFps = 0.0f;

    N64::OSContPad pad{};
    int crosshairX = 0;
    int crosshairY = 0;

    // 10. Bucle Principal
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

        // Actualizar mira
        crosshairX += pad.stick_x / 20;
        crosshairY += pad.stick_y / 20;
        if (crosshairX > 140) crosshairX = 140;
        if (crosshairX < -140) crosshairX = -140;
        if (crosshairY > 100) crosshairY = 100;
        if (crosshairY < -100) crosshairY = -100;

        // Renderizar frame interactivo en RDRAM
        bool buttonA = (pad.button & N64::Buttons::CONT_A) != 0;
        bool buttonB = (pad.button & N64::Buttons::CONT_B) != 0;
        drawInteractiveFrame(fbVaddr, vi.getWidth(), vi.getHeight(), SDL_GetTicks(), crosshairX, crosshairY, buttonA, buttonB);

        // Medidor de FPS
        frameCount++;
        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - lastFpsUpdate >= 500) {
            currentFps = (frameCount * 1000.0f) / (currentTicks - lastFpsUpdate);
            std::string title = "Conker's Bad Fur Day: Recompiled | FPS: " + 
                                std::to_string(static_cast<int>(currentFps)) + 
                                " | Stick: (" + std::to_string(pad.stick_x) + ", " + std::to_string(pad.stick_y) + ") " +
                                (buttonA ? "[A] " : "") + (buttonB ? "[B] " : "");
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsUpdate = currentTicks;
        }

        // Copiar RDRAM -> Textura GPU
        vi.updateTexture(viTexture);

        // Limpiar fondo
        SDL_SetRenderDrawColor(renderer, 15, 18, 22, 255);
        SDL_RenderClear(renderer);

        // Aspect ratio 4:3
        int currentWinW, currentWinH;
        SDL_GetWindowSize(window, &currentWinW, &currentWinH);

        float targetAspect = 4.0f / 3.0f;
        int renderW = currentWinW;
        int renderH = static_cast<int>(renderW / targetAspect);

        if (renderH > currentWinH) {
            renderH = currentWinH;
            renderW = static_cast<int>(renderH * targetAspect);
        }

        SDL_Rect dstRect = {
            (currentWinW - renderW) / 2,
            (currentWinH - renderH) / 2,
            renderW,
            renderH
        };

        SDL_RenderCopy(renderer, viTexture, nullptr, &dstRect);
        SDL_RenderPresent(renderer);
    }

    // Guardar estado del chip EEPROM al salir del juego
    N64::SaveSystem::getInstance().saveEEPROM();

    // Limpieza
    N64::InputManager::getInstance().shutdown();
    if (viTexture) SDL_DestroyTexture(viTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "[Exit] Conker Recompiled closed cleanly." << std::endl;
    return 0;
}
