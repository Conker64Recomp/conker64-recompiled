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
    ofn.lpstrTitle = "Selecciona tu ROM de Conker's Bad Fur Day (USA NTSC)";
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

// Renderiza la pantalla del Launcher con diseno estilizado tematico de Conker / Taberna
void renderNoRomLauncher(SDL_Renderer* renderer, int winW, int winH, float animTime) {
    // 1. Fondo oscuro estilo Taberna (madera oscura y degradado)
    for (int y = 0; y < winH; ++y) {
        float t = static_cast<float>(y) / static_cast<float>(winH);
        uint8_t bgR = static_cast<uint8_t>(18 + t * 10);
        uint8_t bgG = static_cast<uint8_t>(12 + t * 6);
        uint8_t bgB = static_cast<uint8_t>(10 + t * 4);
        SDL_SetRenderDrawColor(renderer, bgR, bgG, bgB, 255);
        SDL_RenderDrawLine(renderer, 0, y, winW, y);
    }

    // Lineas decorativas sutiles de tablones de madera
    SDL_SetRenderDrawColor(renderer, 32, 22, 16, 255);
    int plankH = 48;
    for (int y = 0; y < winH; y += plankH) {
        SDL_RenderDrawLine(renderer, 0, y, winW, y);
    }

    // 2. Panel Central Estilo Cartel de Taberna / Conker
    int panelW = std::min(winW - 100, 780);
    int panelH = std::min(winH - 100, 480);
    int panelX = (winW - panelW) / 2;
    int panelY = (winH - panelH) / 2;

    // Sombra proyectada
    SDL_Rect shadowRect = { panelX + 8, panelY + 8, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 6, 4, 3, 220);
    SDL_RenderFillRect(renderer, &shadowRect);

    // Fondo del panel principal
    SDL_Rect panelRect = { panelX, panelY, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 28, 18, 14, 255);
    SDL_RenderFillRect(renderer, &panelRect);

    // Borde de madera interior
    SDL_Rect woodInner = { panelX + 4, panelY + 4, panelW - 8, panelH - 8 };
    SDL_SetRenderDrawColor(renderer, 44, 28, 20, 255);
    SDL_RenderDrawRect(renderer, &woodInner);

    // Pulso animado de color naranja / dorado Conker
    float pulse = (std::sin(animTime * 3.5f) + 1.0f) * 0.5f;
    uint8_t goldR = static_cast<uint8_t>(235 + pulse * 20);
    uint8_t goldG = static_cast<uint8_t>(145 + pulse * 45);
    uint8_t goldB = static_cast<uint8_t>(25 + pulse * 30);

    // Marco exterior brillante
    SDL_SetRenderDrawColor(renderer, goldR, goldG, goldB, 255);
    SDL_RenderDrawRect(renderer, &panelRect);

    // Remaches / Tachuelas doradas en las esquinas del cartel
    int rivetOffset = 10;
    int rivetSize = 6;
    SDL_Rect r1 = { panelX + rivetOffset, panelY + rivetOffset, rivetSize, rivetSize };
    SDL_Rect r2 = { panelX + panelW - rivetOffset - rivetSize, panelY + rivetOffset, rivetSize, rivetSize };
    SDL_Rect r3 = { panelX + rivetOffset, panelY + panelH - rivetOffset - rivetSize, rivetSize, rivetSize };
    SDL_Rect r4 = { panelX + panelW - rivetOffset - rivetSize, panelY + panelH - rivetOffset - rivetSize, rivetSize, rivetSize };
    SDL_SetRenderDrawColor(renderer, 250, 190, 40, 255);
    SDL_RenderFillRect(renderer, &r1);
    SDL_RenderFillRect(renderer, &r2);
    SDL_RenderFillRect(renderer, &r3);
    SDL_RenderFillRect(renderer, &r4);

    // 3. Encabezado del Panel: "CONKER64: RECOMPILED"
    SDL_Rect headerRect = { panelX + 8, panelY + 8, panelW - 16, 60 };
    SDL_SetRenderDrawColor(renderer, 42, 24, 18, 255);
    SDL_RenderFillRect(renderer, &headerRect);
    SDL_SetRenderDrawColor(renderer, goldR, goldG, goldB, 255);
    SDL_RenderDrawLine(renderer, panelX + 8, panelY + 68, panelX + panelW - 8, panelY + 68);

    // Adorno central en el encabezado (Insignia Corona / Jarra de Cerveza)
    int mugCX = panelX + panelW / 2;
    int mugCY = panelY + 36;
    int mugW = 20, mugH = 24;
    SDL_Rect mugBody = { mugCX - mugW / 2, mugCY - mugH / 2, mugW, mugH };
    SDL_SetRenderDrawColor(renderer, 245, 175, 30, 255);
    SDL_RenderFillRect(renderer, &mugBody);
    // Espuma animada sobre la jarra
    int foamH = 5 + static_cast<int>(pulse * 3);
    SDL_Rect foamRect = { mugCX - mugW / 2 - 2, mugCY - mugH / 2 - foamH, mugW + 4, foamH };
    SDL_SetRenderDrawColor(renderer, 255, 255, 240, 255);
    SDL_RenderFillRect(renderer, &foamRect);
    // Asa de la jarra
    SDL_RenderDrawLine(renderer, mugCX + mugW / 2, mugCY - 6, mugCX + mugW / 2 + 6, mugCY - 6);
    SDL_RenderDrawLine(renderer, mugCX + mugW / 2 + 6, mugCY - 6, mugCX + mugW / 2 + 6, mugCY + 6);
    SDL_RenderDrawLine(renderer, mugCX + mugW / 2, mugCY + 6, mugCX + mugW / 2 + 6, mugCY + 6);

    // Burbujas ascendentes animadas
    for (int i = 0; i < 5; ++i) {
        float bY = std::fmod(animTime * 25.0f + i * 14.0f, 40.0f);
        int bx = mugCX - 8 + (i * 4);
        int by = mugCY + 8 - static_cast<int>(bY);
        if (by > mugCY - 8) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 200, 200);
            SDL_RenderDrawPoint(renderer, bx, by);
        }
    }

    // 4. Zona Central de Drop (Área interactiva para arrastrar la ROM)
    int dropMarginX = 35;
    int dropW = panelW - dropMarginX * 2;
    int dropH = panelH - 180;
    int dropX = panelX + dropMarginX;
    int dropY = panelY + 85;

    SDL_Rect dropZone = { dropX, dropY, dropW, dropH };
    SDL_SetRenderDrawColor(renderer, 16, 10, 8, 255);
    SDL_RenderFillRect(renderer, &dropZone);

    // Borde discontinuo animado de la zona de drop
    SDL_SetRenderDrawColor(renderer, static_cast<uint8_t>(goldR * 0.8f), static_cast<uint8_t>(goldG * 0.8f), 40, 255);
    int dash = 14;
    int shift = static_cast<int>(animTime * 18.0f) % (dash * 2);
    for (int x = dropX + shift; x < dropX + dropW; x += dash * 2) {
        SDL_RenderDrawLine(renderer, x, dropY, std::min(x + dash, dropX + dropW), dropY);
        SDL_RenderDrawLine(renderer, x, dropY + dropH, std::min(x + dash, dropX + dropW), dropY + dropH);
    }
    for (int y = dropY + shift; y < dropY + dropH; y += dash * 2) {
        SDL_RenderDrawLine(renderer, dropX, y, dropX, std::min(y + dash, dropY + dropH));
        SDL_RenderDrawLine(renderer, dropX + dropW, y, dropX + dropW, std::min(y + dash, dropY + dropH));
    }

    // Esquinas con corchetes destacados
    int cornerLen = 22;
    SDL_SetRenderDrawColor(renderer, 255, 195, 45, 255);
    // Superior Izquierda
    SDL_RenderDrawLine(renderer, dropX, dropY, dropX + cornerLen, dropY);
    SDL_RenderDrawLine(renderer, dropX, dropY, dropX, dropY + cornerLen);
    // Superior Derecha
    SDL_RenderDrawLine(renderer, dropX + dropW - cornerLen, dropY, dropX + dropW, dropY);
    SDL_RenderDrawLine(renderer, dropX + dropW, dropY, dropX + dropW, dropY + cornerLen);
    // Inferior Izquierda
    SDL_RenderDrawLine(renderer, dropX, dropY + dropH - cornerLen, dropX, dropY + dropH);
    SDL_RenderDrawLine(renderer, dropX, dropY + dropH, dropX + cornerLen, dropY + dropH);
    // Inferior Derecha
    SDL_RenderDrawLine(renderer, dropX + dropW - cornerLen, dropY + dropH, dropX + dropW, dropY + dropH);
    SDL_RenderDrawLine(renderer, dropX + dropW, dropY + dropH - cornerLen, dropX + dropW, dropY + dropH);

    // Icono animado de Cartucho N64 en el centro de la zona de drop
    int iconCX = dropX + dropW / 2;
    int iconCY = dropY + dropH / 2 - 25;
    int cartW = 54, cartH = 46;
    float bounce = std::sin(animTime * 4.0f) * 4.0f;
    int cartY = iconCY - cartH / 2 + static_cast<int>(bounce);

    SDL_Rect cartBody = { iconCX - cartW / 2, cartY, cartW, cartH };
    SDL_SetRenderDrawColor(renderer, 70, 75, 85, 255);
    SDL_RenderFillRect(renderer, &cartBody);
    SDL_SetRenderDrawColor(renderer, goldR, goldG, goldB, 255);
    SDL_RenderDrawRect(renderer, &cartBody);

    // Etiqueta del cartucho
    SDL_Rect cartLabel = { iconCX - cartW / 2 + 8, cartY + 8, cartW - 16, cartH - 18 };
    SDL_SetRenderDrawColor(renderer, 220, 80, 20, 255);
    SDL_RenderFillRect(renderer, &cartLabel);

    // Contactos dorados inferiores del cartucho
    SDL_Rect cartPins = { iconCX - cartW / 2 + 12, cartY + cartH, cartW - 24, 4 };
    SDL_SetRenderDrawColor(renderer, 240, 200, 60, 255);
    SDL_RenderFillRect(renderer, &cartPins);

    // 5. Boton de Accion / Busqueda con Tecla [O]
    int btnW = 340;
    int btnH = 44;
    int btnX = dropX + (dropW - btnW) / 2;
    int btnY = dropY + dropH - 58;

    SDL_Rect btnBg = { btnX, btnY, btnW, btnH };
    SDL_SetRenderDrawColor(renderer, 38, 54, 86, 255);
    SDL_RenderFillRect(renderer, &btnBg);
    SDL_SetRenderDrawColor(renderer, 245, 175, 30, 255);
    SDL_RenderDrawRect(renderer, &btnBg);
    SDL_Rect btnInner = { btnX + 2, btnY + 2, btnW - 4, btnH - 4 };
    SDL_SetRenderDrawColor(renderer, 50, 70, 110, 255);
    SDL_RenderDrawRect(renderer, &btnInner);

    // Insignia de tecla [ O ]
    int badgeW = 28, badgeH = 26;
    int badgeX = btnX + 12;
    int badgeY = btnY + (btnH - badgeH) / 2;
    SDL_Rect badgeRect = { badgeX, badgeY, badgeW, badgeH };
    SDL_SetRenderDrawColor(renderer, 255, 195, 40, 255);
    SDL_RenderFillRect(renderer, &badgeRect);
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderDrawRect(renderer, &badgeRect);

    // 6. Pie del Panel con Informacion Legal y Preservacion
    int footerY = panelY + panelH - 75;
    SDL_SetRenderDrawColor(renderer, 42, 26, 18, 255);
    SDL_RenderDrawLine(renderer, panelX + 16, footerY, panelX + panelW - 16, footerY);

    // Barra de estado inferior
    int statusY = winH - 30;
    SDL_SetRenderDrawColor(renderer, 10, 7, 5, 255);
    SDL_Rect statusRect = { 0, statusY, winW, 30 };
    SDL_RenderFillRect(renderer, &statusRect);
    SDL_SetRenderDrawColor(renderer, 60, 40, 25, 255);
    SDL_RenderDrawLine(renderer, 0, statusY, winW, statusY);
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
