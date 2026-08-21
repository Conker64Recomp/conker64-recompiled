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
#include "actor_system.hpp"
#include "intro_sequence.hpp"
#include "audio_rom.hpp"
#include "asset_paths.hpp"

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
    N64::ActorManager::getInstance().init();
    N64::IntroSequence::getInstance().init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "[Error] SDL2 init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    std::cout << "[Init] SDL2 Video & Audio Systems Initialized... OK" << std::endl;

    // Debe resolverse antes de que el RDP cargue mallas y texturas.
    N64::AssetPaths::getInstance().init();

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

    // Abrir un dispositivo de audio SDL2 dedicado para la reproducción de MP3 de la ROM
    SDL_AudioSpec want{}, got{};
    want.freq     = 44100;
    want.format   = AUDIO_S16SYS;
    want.channels = 2;
    want.samples  = 2048;
    want.callback = nullptr; // Usaremos SDL_QueueAudio en vez de callback
    SDL_AudioDeviceID romAudioDev = SDL_OpenAudioDevice(nullptr, 0, &want, &got, 0);
    if (romAudioDev > 0) {
        N64::ROMaudioPlayer::getInstance().init(romAudioDev, got);
        SDL_PauseAudioDevice(romAudioDev, 0);
        std::cout << "[ROM Audio] SDL Audio Device " << romAudioDev << " opened for ROM MP3 playback." << std::endl;
    }

    // Carga de ROM automática
    bool romLoaded = false;
    std::string loadedRomPath;
    // El cargador normaliza z64/v64/n64 por firma, asi que cualquiera sirve.
    std::vector<std::string> searchPaths = {
        "baserom.us.z64", "../baserom.us.z64", "../../baserom.us.z64",
        "baserom.us.n64", "../baserom.us.n64", "../../baserom.us.n64",
        "Conker's Bad Fur Day (USA).z64",
        "../Conker's Bad Fur Day (USA).z64",
        "../../Conker's Bad Fur Day (USA).z64",
        "Conker's Bad Fur Day (USA).n64",
        "../Conker's Bad Fur Day (USA).n64",
        "../../Conker's Bad Fur Day (USA).n64"
    };

    for (const auto& path : searchPaths) {
        if (std::filesystem::exists(path)) {
            if (N64::ROMLoader::loadROM(path)) {
                romLoaded = true;
                loadedRomPath = path;

                // Inicializar decodificador de audio de la ROM real
                const uint8_t* romBuf = N64::AssetDecoder::getInstance().getROMBuffer();
                size_t romBufSize     = N64::AssetDecoder::getInstance().getROMSize();
                if (romBuf && romBufSize > 0) {
                    N64::ROMaudioDecoder::getInstance().init(romBuf, romBufSize);
                    std::cout << "[ROM Audio] Indexed " << N64::ROMaudioDecoder::getInstance().trackCount()
                              << " real MP3 tracks from assets16." << std::endl;
                    // Reproducir Track 0: Música de la intro de Conker's Bad Fur Day
                    N64::ROMaudioPlayer::getInstance().playTrack(0);
                }

                N64::MIPSRecompiler::getInstance().executeBootFunction();
                loadRealTextureFromROM(renderer);
                break;
            }
        }
    }

    uint32_t frameCount = 0;
    uint32_t lastFpsUpdate = SDL_GetTicks();
    uint32_t lastFpsLog = SDL_GetTicks();
    float currentFps = 0.0f;
    float rotationAngle = 0.0f;
    N64::OSContPad pad{};

    int currentTextureIdx = 0;
    std::string currentTextureName = "Hojas y Vegetacion (assets00:0)";

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
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
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
                // Tab, no Espacio: Espacio ya es saltar, y compartirlo hacia que
                // cada salto ciclara las texturas (con autorepeat, en bucle).
                else if (event.key.keysym.sym == SDLK_TAB && event.key.repeat == 0) {
                    currentTextureIdx = (currentTextureIdx + 1) % 9;
                    int tw = 64, th = 64;
                    auto texData = N64::AssetDecoder::getInstance().loadTextureByIndex(currentTextureIdx, tw, th, currentTextureName);
                    if (!texData.empty()) {
                        N64::RDPProcessor::getInstance().loadRealTexture(renderer, texData.data(), tw, th);
                    }
                }
                else if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9) {
                    currentTextureIdx = event.key.keysym.sym - SDLK_1;
                    int tw = 64, th = 64;
                    auto texData = N64::AssetDecoder::getInstance().loadTextureByIndex(currentTextureIdx, tw, th, currentTextureName);
                    if (!texData.empty()) {
                        N64::RDPProcessor::getInstance().loadRealTexture(renderer, texData.data(), tw, th);
                    }
                }
            }
        }

        N64::InputManager::getInstance().poll(pad);

        const float dt = 1.0f / 60.0f;

        // Control de rotación orbital de cámara con C-Buttons / Teclas Q y E
        float camInputX = 0.0f;
        const Uint8* keyState = SDL_GetKeyboardState(nullptr);
        if (keyState[SDL_SCANCODE_Q] || (pad.button & N64::Buttons::CONT_C)) camInputX -= 1.0f;
        if (keyState[SDL_SCANCODE_E] || (pad.button & N64::Buttons::CONT_F)) camInputX += 1.0f;

        // El yaw se integra ANTES de mover al jugador, para que su input quede
        // orientado respecto a la cámara vigente en este mismo frame.
        N64::RDPProcessor::getInstance().advanceCameraYaw(camInputX, dt);
        N64::ActorManager::getInstance().updatePlayer(
            pad, dt, N64::RDPProcessor::getInstance().getCameraYaw());

        // Ejecución de la lógica del hilo principal del juego recompilado
        N64::MIPSRecompiler::getInstance().updateGameLogic(dt);

        const auto& player = N64::ActorManager::getInstance().getPlayer();

        frameCount++;
        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - lastFpsUpdate >= 500) {
            currentFps = (frameCount * 1000.0f) / (currentTicks - lastFpsUpdate);
            std::string title = romLoaded
                ? "Conker64: Recompiled | [" + std::to_string(currentTextureIdx + 1) + "/9] " + currentTextureName + " | Cam: Q/E | FPS: " + std::to_string(static_cast<int>(currentFps))
                : "Conker64: Recompiled | ESPERANDO ROM (Arrastra tu .z64 o presiona O)";
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsUpdate = currentTicks;

            // Traza de rendimiento a bajo ritmo: el renderer es software y el
            // coste depende mucho de cuantos triangulos traiga el nivel.
            if (currentTicks - lastFpsLog >= 3000) {
                std::cout << "[Perf] " << static_cast<int>(currentFps) << " FPS" << std::endl;
                lastFpsLog = currentTicks;
            }
        }

        int winW = 960, winH = 540;
        SDL_GetWindowSize(window, &winW, &winH);

        // Actualización de la cinemática de Intro oficial
        N64::IntroSequence::getInstance().update(dt);

        if (!N64::IntroSequence::getInstance().isGameplayActive()) {
            N64::IntroSequence::getInstance().render(renderer, winW, winH);
        } else {
            // Fondo azul cielo estilo N64
            SDL_SetRenderDrawColor(renderer, 70, 130, 200, 255);
            SDL_RenderClear(renderer);

            N64::RDPProcessor::getInstance().processDisplayList(0, renderer, winW, winH, player, dt);
        }

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
