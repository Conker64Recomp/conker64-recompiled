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
#include "asset_decoder.hpp"
#include "asset_manager.hpp"

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
    ofn.lpstrTitle = "Selecciona tu ROM de Conker's Bad Fur Day";
    if (GetOpenFileNameA(&ofn)) return std::string(filename);
#endif
    return "";
}

// Carga y decomprime la primera textura real de assets00 y la sube a la GPU
void loadRealTextureFromROM(SDL_Renderer* renderer) {
    int texW = 96, texH = 110;
    auto texData = N64::AssetDecoder::getInstance().loadFirstTexture(texW, texH);
    if (!texData.empty()) {
        N64::RDPProcessor::getInstance().loadRealTexture(renderer, texData.data(), texW, texH);
    }
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    std::cout << "========================================" << std::endl;
    std::cout << " CONKER64: RECOMPILED (NATIVE PC PORT)" << std::endl;
    std::cout << " Engine: SDL2 + Rareware Asset Decoder" << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "[Paths] Saves: " << N64::PathManager::getAppDataPath() << std::endl;
    std::cout << "[Paths] Cache: " << N64::PathManager::getCachePath() << std::endl;

    N64::Memory::getInstance().init();
    N64::SaveSystem::getInstance().init();
    N64::MIPSRecompiler::getInstance().init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "[Error] SDL2 init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    std::cout << "[Init] SDL2 Video & Audio Systems Initialized... OK" << std::endl;

    N64::AudioManager::getInstance().init(44100);
    N64::InputManager::getInstance().init();

    int windowWidth = 1280, windowHeight = 720;
    SDL_Window* window = SDL_CreateWindow(
        "Conker64: Recompiled - Arrastra tu ROM (.z64) o Presiona [O] para Buscar",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) { SDL_Quit(); return 1; }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    N64::RDPProcessor::getInstance().init(renderer);

    // Carga de ROM automática
    bool romLoaded = false;
    std::string loadedRomPath;
    std::vector<std::string> searchPaths = {
        "baserom.us.z64", "../baserom.us.z64", "../../baserom.us.z64",
        "Conker's Bad Fur Day (USA).z64"
    };

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            if (N64::ROMLoader::loadROM(path)) {
                romLoaded = true;
                loadedRomPath = path;
                N64::MIPSRecompiler::getInstance().executeBootFunction();
                loadRealTextureFromROM(renderer);
                break;
            }
        }
    }

    uint32_t frameCount = 0;
    uint32_t lastFpsUpdate = SDL_GetTicks();
    float currentFps = 0.0f;
    float rotationAngle = 0.0f;
    N64::OSContPad pad{};

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;
            else if (event.type == SDL_DROPFILE) {
                char* droppedFile = event.drop.file;
                std::cout << "[Launcher] ROM Arrastrada: " << droppedFile << std::endl;
                if (N64::ROMLoader::loadROM(droppedFile)) {
                    romLoaded = true;
                    loadedRomPath = droppedFile;
                    N64::MIPSRecompiler::getInstance().executeBootFunction();
                    loadRealTextureFromROM(renderer);
                }
                SDL_free(droppedFile);
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) isRunning = false;
                else if (event.key.keysym.sym == SDLK_o) {
                    std::string selected = openFileDialog();
                    if (!selected.empty() && N64::ROMLoader::loadROM(selected)) {
                        romLoaded = true;
                        loadedRomPath = selected;
                        N64::MIPSRecompiler::getInstance().executeBootFunction();
                        loadRealTextureFromROM(renderer);
                    }
                }
                else if (event.key.keysym.sym == SDLK_t) {
                    N64::AudioManager::getInstance().playBootJingle();
                }
            }
        }

        N64::InputManager::getInstance().poll(pad);
        float rotSpeed = 1.2f;
        if (std::abs(pad.stick_x) > 10) rotSpeed = (pad.stick_x / 80.0f) * 4.0f;
        rotationAngle += rotSpeed;
        if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
        if (rotationAngle < 0.0f) rotationAngle += 360.0f;

        frameCount++;
        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - lastFpsUpdate >= 500) {
            currentFps = (frameCount * 1000.0f) / (currentTicks - lastFpsUpdate);
            std::string title = romLoaded
                ? "Conker64: Recompiled | Rareware Assets Active | FPS: " + std::to_string(static_cast<int>(currentFps))
                : "Conker64: Recompiled | ESPERANDO ROM (Arrastra tu .z64 o presiona O)";
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsUpdate = currentTicks;
        }

        SDL_SetRenderDrawColor(renderer, 15, 18, 24, 255);
        SDL_RenderClear(renderer);

        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);
        N64::RDPProcessor::getInstance().processDisplayList(0, renderer, winW, winH, rotationAngle);

        SDL_RenderPresent(renderer);
    }

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
// force  
