#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>
#include <cmath>

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
    ofn.lpstrTitle = "Selecciona tu ROM de Conker's Bad Fur Day (USA)";
    if (GetOpenFileNameA(&ofn)) return std::string(filename);
#endif
    return "";
}

// Carga y descomprime la primera textura real de assets00 y la sube a la GPU
void loadRealTextureFromROM(SDL_Renderer* renderer) {
    int texW = 96, texH = 110;
    auto texData = N64::AssetDecoder::getInstance().loadFirstTexture(texW, texH);
    if (!texData.empty()) {
        N64::RDPProcessor::getInstance().loadRealTexture(renderer, texData.data(), texW, texH);
    }
}

// Inicializa todos los subsistemas del juego una vez que se valida la ROM
bool startLoadedGame(const std::string& path, SDL_Renderer* renderer) {
    std::cout << "[Launcher] Cargando ROM: " << path << std::endl;
    if (!N64::ROMLoader::loadROM(path)) {
        std::cerr << "[Launcher] Error: La ROM no es valida o no se pudo leer." << std::endl;
        return false;
    }

    const uint8_t* romBuf = N64::AssetDecoder::getInstance().getROMBuffer();
    size_t romBufSize     = N64::AssetDecoder::getInstance().getROMSize();
    if (romBuf && romBufSize > 0) {
        N64::ROMaudioDecoder::getInstance().init(romBuf, romBufSize);
        std::cout << "[ROM Audio] Indexados " << N64::ROMaudioDecoder::getInstance().trackCount()
                  << " tracks de audio reales desde assets16." << std::endl;
    }

    N64::MIPSRecompiler::getInstance().executeBootFunction();
    loadRealTextureFromROM(renderer);
    N64::IntroSequence::getInstance().init();
    std::cout << "[Launcher] ROM cargada con exito. Iniciando experiencia de juego..." << std::endl;
    return true;
}

// Renderiza la pantalla del Launcher cuando NO hay una ROM cargada
void renderNoRomLauncher(SDL_Renderer* renderer, int winW, int winH, float animTime) {
    // Fondo oscuro con degradado
    SDL_SetRenderDrawColor(renderer, 15, 18, 24, 255);
    SDL_RenderClear(renderer);

    // Cuadrícula sutil de fondo
    SDL_SetRenderDrawColor(renderer, 25, 30, 40, 255);
    int gridStep = 40;
    for (int x = 0; x < winW; x += gridStep) {
        SDL_RenderDrawLine(renderer, x, 0, x, winH);
    }
    for (int y = 0; y < winH; y += gridStep) {
        SDL_RenderDrawLine(renderer, 0, y, winW, y);
    }

    // Panel Central (Caja de Drop / Launcher)
    int boxW = std::min(winW - 80, 720);
    int boxH = std::min(winH - 80, 420);
    int boxX = (winW - boxW) / 2;
    int boxY = (winH - boxH) / 2;

    // Sombra del panel
    SDL_Rect shadowRect = { boxX + 6, boxY + 6, boxW, boxH };
    SDL_SetRenderDrawColor(renderer, 5, 6, 8, 200);
    SDL_RenderFillRect(renderer, &shadowRect);

    // Cuerpo del panel
    SDL_Rect bgRect = { boxX, boxY, boxW, boxH };
    SDL_SetRenderDrawColor(renderer, 24, 28, 38, 255);
    SDL_RenderFillRect(renderer, &bgRect);

    // Borde animado con efecto pulso (amarillo/naranja Conker)
    float pulse = (std::sin(animTime * 3.0f) + 1.0f) * 0.5f;
    uint8_t borderR = static_cast<uint8_t>(220 + pulse * 35);
    uint8_t borderG = static_cast<uint8_t>(140 + pulse * 60);
    uint8_t borderB = static_cast<uint8_t>(20 + pulse * 40);
    SDL_SetRenderDrawColor(renderer, borderR, borderG, borderB, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    SDL_Rect bgRectInner = { boxX + 1, boxY + 1, boxW - 2, boxH - 2 };
    SDL_RenderDrawRect(renderer, &bgRectInner);

    // Barra superior del panel
    SDL_Rect headerRect = { boxX, boxY, boxW, 50 };
    SDL_SetRenderDrawColor(renderer, 35, 42, 56, 255);
    SDL_RenderFillRect(renderer, &headerRect);
    SDL_SetRenderDrawColor(renderer, borderR, borderG, borderB, 255);
    SDL_RenderDrawLine(renderer, boxX, boxY + 50, boxX + boxW, boxY + 50);

    // Zona de Drop interior
    int dropW = boxW - 60;
    int dropH = boxH - 120;
    int dropX = boxX + 30;
    int dropY = boxY + 70;

    SDL_Rect dropRect = { dropX, dropY, dropW, dropH };
    SDL_SetRenderDrawColor(renderer, 18, 22, 30, 255);
    SDL_RenderFillRect(renderer, &dropRect);

    // Borde discontinuo de la zona de drop
    SDL_SetRenderDrawColor(renderer, 70, 90, 120, 255);
    int dashLen = 12;
    for (int x = dropX; x < dropX + dropW; x += dashLen * 2) {
        SDL_RenderDrawLine(renderer, x, dropY, std::min(x + dashLen, dropX + dropW), dropY);
        SDL_RenderDrawLine(renderer, x, dropY + dropH, std::min(x + dashLen, dropX + dropW), dropY + dropH);
    }
    for (int y = dropY; y < dropY + dropH; y += dashLen * 2) {
        SDL_RenderDrawLine(renderer, dropX, y, dropX, std::min(y + dashLen, dropY + dropH));
        SDL_RenderDrawLine(renderer, dropX + dropW, y, dropX + dropW, std::min(y + dashLen, dropY + dropH));
    }

    // Icono N64 / Cartucho en el centro de la zona de drop
    int iconCX = dropX + dropW / 2;
    int iconCY = dropY + dropH / 2 - 20;
    int iconSz = 36;
    SDL_Rect iconRect = { iconCX - iconSz / 2, iconCY - iconSz / 2, iconSz, iconSz };
    SDL_SetRenderDrawColor(renderer, borderR, borderG, borderB, 255);
    SDL_RenderDrawRect(renderer, &iconRect);

    // Líneas decorativas del icono
    SDL_RenderDrawLine(renderer, iconCX - iconSz / 2, iconCY, iconCX + iconSz / 2, iconCY);
    SDL_RenderDrawLine(renderer, iconCX, iconCY - iconSz / 2, iconCX, iconCY + iconSz / 2);

    // Botón / Indicador "[ PRESIONA 'O' PARA BUSCAR ROM ]"
    int btnW = 320;
    int btnH = 40;
    int btnX = dropX + (dropW - btnW) / 2;
    int btnY = dropY + dropH - 55;
    SDL_Rect btnRect = { btnX, btnY, btnW, btnH };
    SDL_SetRenderDrawColor(renderer, 45, 60, 85, 255);
    SDL_RenderFillRect(renderer, &btnRect);
    SDL_SetRenderDrawColor(renderer, 240, 180, 50, 255);
    SDL_RenderDrawRect(renderer, &btnRect);
}

int main(int argc, char** argv) {
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "[Error] SDL2 init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    std::cout << "[Init] SDL2 Video & Audio Systems Initialized... OK" << std::endl;

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
    want.callback = nullptr;
    SDL_AudioDeviceID romAudioDev = SDL_OpenAudioDevice(nullptr, 0, &want, &got, 0);
    if (romAudioDev > 0) {
        N64::ROMaudioPlayer::getInstance().init(romAudioDev, got);
        SDL_PauseAudioDevice(romAudioDev, 0);
        std::cout << "[ROM Audio] SDL Audio Device " << romAudioDev << " ready." << std::endl;
    }

    // Comprobación de ROM (vía argumento CLI o búsqueda automática)
    bool romLoaded = false;
    std::string loadedRomPath;

    // 1. Argumento de línea de comandos (e.g. Conker.exe "C:\Games\baserom.us.z64")
    if (argc > 1 && argv[1]) {
        std::string cliPath = argv[1];
        if (std::filesystem::exists(cliPath)) {
            if (startLoadedGame(cliPath, renderer)) {
                romLoaded = true;
                loadedRomPath = cliPath;
            }
        }
    }

    // 2. Búsqueda automática en rutas locales si no se cargó por CLI
    if (!romLoaded) {
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
                if (startLoadedGame(path, renderer)) {
                    romLoaded = true;
                    loadedRomPath = path;
                    break;
                }
            }
        }
    }

    if (!romLoaded) {
        std::cout << "[Launcher] No se detecto ROM automatica. Esperando que el usuario arrastre su ROM o presione [O]..." << std::endl;
    }

    uint32_t frameCount = 0;
    uint32_t lastFpsUpdate = SDL_GetTicks();
    uint32_t lastFpsLog = SDL_GetTicks();
    float currentFps = 0.0f;
    float launcherAnimTime = 0.0f;
    N64::OSContPad pad{};

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;
            else if (event.type == SDL_DROPFILE) {
                char* droppedFile = event.drop.file;
                std::cout << "[Launcher] ROM Arrastrada: " << droppedFile << std::endl;
                if (startLoadedGame(droppedFile, renderer)) {
                    romLoaded = true;
                    loadedRomPath = droppedFile;
                }
                SDL_free(droppedFile);
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                else if (event.key.keysym.sym == SDLK_o) {
                    std::string selected = openFileDialog();
                    if (!selected.empty() && startLoadedGame(selected, renderer)) {
                        romLoaded = true;
                        loadedRomPath = selected;
                    }
                }
            }
        }

        const float dt = 1.0f / 60.0f;
        launcherAnimTime += dt;

        int winW = 1280, winH = 720;
        SDL_GetWindowSize(window, &winW, &winH);

        frameCount++;
        uint32_t currentTicks = SDL_GetTicks();
        if (currentTicks - lastFpsUpdate >= 500) {
            currentFps = (frameCount * 1000.0f) / (currentTicks - lastFpsUpdate);
            std::string title = romLoaded
                ? "Conker64: Recompiled | Cam: Q/E | FPS: " + std::to_string(static_cast<int>(currentFps))
                : "Conker64: Recompiled | ESPERANDO ROM (Arrastra tu .z64 o presiona O)";
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
            lastFpsUpdate = currentTicks;

            if (romLoaded && currentTicks - lastFpsLog >= 3000) {
                std::cout << "[Perf] " << static_cast<int>(currentFps) << " FPS" << std::endl;
                lastFpsLog = currentTicks;
            }
        }

        // =========================================================================
        // CONTROL DE FLUJO: SI NO HAY ROM CARGADA, NO EJECUTAR EL JUEGO NI EL AUDIO
        // =========================================================================
        if (!romLoaded) {
            // Renderizar pantalla de espera del Launcher (0 audio, 0 assets in-game)
            renderNoRomLauncher(renderer, winW, winH, launcherAnimTime);
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }

        // =========================================================================
        // FLUJO DE JUEGO ACTIVO (SOLO CON ROM VALIDA)
        // =========================================================================
        N64::InputManager::getInstance().poll(pad);

        // Control de rotación orbital de cámara con C-Buttons / Teclas Q y E
        float camInputX = 0.0f;
        const Uint8* keyState = SDL_GetKeyboardState(nullptr);
        if (keyState[SDL_SCANCODE_Q] || (pad.button & N64::Buttons::CONT_C)) camInputX -= 1.0f;
        if (keyState[SDL_SCANCODE_E] || (pad.button & N64::Buttons::CONT_F)) camInputX += 1.0f;

        // El yaw se integra ANTES de mover al jugador
        N64::RDPProcessor::getInstance().advanceCameraYaw(camInputX, dt);
        N64::ActorManager::getInstance().updatePlayer(
            pad, dt, N64::RDPProcessor::getInstance().getCameraYaw());

        // Lógica del juego recompilado
        N64::MIPSRecompiler::getInstance().updateGameLogic(dt);

        const auto& player = N64::ActorManager::getInstance().getPlayer();

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
