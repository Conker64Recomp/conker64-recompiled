#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>
#include <cmath>
#include <algorithm>

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

// ─── EMBEDDED BITMAP FONT SYSTEM (8x8 ASCII FONT) ───────────────────────────
static const uint8_t font8x8_basic[128][8] = {
    {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
    {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space (32)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // !
    {0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
    {0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00}, // #
    {0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00}, // $
    {0x00, 0x63, 0x66, 0x0C, 0x18, 0x33, 0x63, 0x00}, // %
    {0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00}, // &
    {0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00}, // '
    {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00}, // (
    {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00}, // )
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // *
    {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00}, // +
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x08}, // ,
    {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00}, // -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // .
    {0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00}, // /
    {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00}, // 0
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, // 1
    {0x3C, 0x66, 0x06, 0x1C, 0x30, 0x66, 0x7E, 0x00}, // 2
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, // 3
    {0x0C, 0x1C, 0x3C, 0x6C, 0xFE, 0x0C, 0x0C, 0x00}, // 4
    {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00}, // 5
    {0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00}, // 6
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00}, // 7
    {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00}, // 8
    {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00}, // 9
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00}, // :
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x08}, // ;
    {0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00}, // <
    {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00}, // =
    {0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00}, // >
    {0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00}, // ?
    {0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x62, 0x3C, 0x00}, // @
    {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00}, // A
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00}, // B
    {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, // C
    {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, // D
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00}, // E
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00}, // F
    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3A, 0x00}, // G
    {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, // H
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // I
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00}, // J
    {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, // K
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, // L
    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, // M
    {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00}, // N
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // O
    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}, // P
    {0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00}, // Q
    {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00}, // R
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}, // S
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // T
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // U
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00}, // V
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // W
    {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00}, // X
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}, // Y
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00}, // Z
    {0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00}, // [
    {0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00}, // \
    {0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00}, // ]
    {0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00}, // ^
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00}, // _
    {0x18, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00}, // `
    // a-z
    {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00},
    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00},
    {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00},
    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3A, 0x00},
    {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00},
    {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00},
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00},
    {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
    {0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00},
    {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00},
    {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},
    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
    {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00},
    {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},
    {0x0E, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0E, 0x00},
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    {0x70, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x70, 0x00},
    {0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

void drawChar(SDL_Renderer* renderer, char c, int x, int y, int scale, SDL_Color color) {
    uint8_t idx = static_cast<uint8_t>(c);
    if (idx >= 128) idx = 32;
    const uint8_t* glyph = font8x8_basic[idx];

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int row = 0; row < 8; ++row) {
        uint8_t b = glyph[row];
        for (int col = 0; col < 8; ++col) {
            if (b & (0x80 >> col)) {
                if (scale == 1) {
                    SDL_RenderDrawPoint(renderer, x + col, y + row);
                } else {
                    SDL_Rect pixel = { x + col * scale, y + row * scale, scale, scale };
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
    }
}

void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, int scale, SDL_Color color, bool shadow = true) {
    if (shadow) {
        SDL_Color sc = { 0, 0, 0, 200 };
        for (size_t i = 0; i < text.size(); ++i) {
            drawChar(renderer, text[i], x + static_cast<int>(i) * 8 * scale + scale, y + scale, scale, sc);
        }
    }
    for (size_t i = 0; i < text.size(); ++i) {
        drawChar(renderer, text[i], x + static_cast<int>(i) * 8 * scale, y, scale, color);
    }
}

int getTextWidth(const std::string& text, int scale) {
    return static_cast<int>(text.size()) * 8 * scale;
}

void drawCenteredText(SDL_Renderer* renderer, const std::string& text, int cx, int y, int scale, SDL_Color color, bool shadow = true) {
    int w = getTextWidth(text, scale);
    drawText(renderer, text, cx - w / 2, y, scale, color, shadow);
}

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

void loadRealTextureFromROM(SDL_Renderer* renderer) {
    int texW = 96, texH = 110;
    auto texData = N64::AssetDecoder::getInstance().loadFirstTexture(texW, texH);
    if (!texData.empty()) {
        N64::RDPProcessor::getInstance().loadRealTexture(renderer, texData.data(), texW, texH);
    }
}

// Pantalla de carga animada (Feedback instantáneo para cero lag percibido)
void renderLoadingScreen(SDL_Renderer* renderer, int winW, int winH, const std::string& message) {
    SDL_SetRenderDrawColor(renderer, 10, 14, 22, 255);
    SDL_RenderClear(renderer);

    int cx = winW / 2;
    int cy = winH / 2;

    // Spinner animado
    SDL_Rect box = { cx - 220, cy - 60, 440, 120 };
    SDL_SetRenderDrawColor(renderer, 24, 30, 44, 255);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 245, 158, 11, 255);
    SDL_RenderDrawRect(renderer, &box);

    drawCenteredText(renderer, "CARGANDO Y DESCOMPRIMIENDO", cx, cy - 35, 2, { 245, 158, 11, 255 }, true);
    drawCenteredText(renderer, message, cx, cy + 5, 1, { 255, 255, 255, 255 }, true);
    drawCenteredText(renderer, "POR FAVOR ESPERA UN MOMENTO...", cx, cy + 25, 1, { 148, 163, 184, 255 }, true);

    SDL_RenderPresent(renderer);
}

bool startLoadedGame(const std::string& path, SDL_Renderer* renderer, int winW, int winH) {
    std::cout << "[Launcher] Cargando ROM: " << path << std::endl;
    renderLoadingScreen(renderer, winW, winH, "DESCOMPRIMIENDO 508 SUBSEGMENTOS RZIP...");

    if (!N64::ROMLoader::loadROM(path)) {
        std::cerr << "[Launcher] Error: La ROM no es valida o no se pudo leer." << std::endl;
        return false;
    }

    const uint8_t* romBuf = N64::AssetDecoder::getInstance().getROMBuffer();
    size_t romBufSize     = N64::AssetDecoder::getInstance().getROMSize();
    if (romBuf && romBufSize > 0) {
        N64::ROMaudioDecoder::getInstance().init(romBuf, romBufSize);
    }

    N64::MIPSRecompiler::getInstance().executeBootFunction();
    loadRealTextureFromROM(renderer);
    N64::IntroSequence::getInstance().init();
    std::cout << "[Launcher] ROM cargada con exito. Iniciando experiencia de juego..." << std::endl;
    return true;
}

// ─── MODERN REACT/TAILWIND-INSPIRED LAUNCHER UI ─────────────────────────────
void renderModernLauncher(SDL_Renderer* renderer, int winW, int winH, float animTime, bool isHoveringDrop, bool isHoveringBtn) {
    // 1. Fondo Slate-950 con degradado
    for (int y = 0; y < winH; ++y) {
        float t = static_cast<float>(y) / static_cast<float>(winH);
        uint8_t bgR = static_cast<uint8_t>(10 + t * 8);   // #0A0E17 -> #121B2A
        uint8_t bgG = static_cast<uint8_t>(14 + t * 11);
        uint8_t bgB = static_cast<uint8_t>(22 + t * 16);
        SDL_SetRenderDrawColor(renderer, bgR, bgG, bgB, 255);
        SDL_RenderDrawLine(renderer, 0, y, winW, y);
    }

    // Grid sutil
    SDL_SetRenderDrawColor(renderer, 30, 41, 59, 120);
    int gridStep = 48;
    for (int x = 0; x < winW; x += gridStep) {
        for (int y = 0; y < winH; y += gridStep) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    // 2. Tarjeta Principal Glassmorphism
    int panelW = std::min(winW - 80, 840);
    int panelH = std::min(winH - 80, 560);
    int panelX = (winW - panelW) / 2;
    int panelY = (winH - panelH) / 2;

    // Sombra proyectada
    SDL_Rect shadow2 = { panelX + 8, panelY + 8, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 2, 5, 10, 190);
    SDL_RenderFillRect(renderer, &shadow2);

    SDL_Rect panelBg = { panelX, panelY, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 20, 26, 38, 255);
    SDL_RenderFillRect(renderer, &panelBg);

    // Pulso animado ámbar
    float pulse = (std::sin(animTime * 3.0f) + 1.0f) * 0.5f;
    uint8_t borderR = static_cast<uint8_t>(217 + pulse * 38);
    uint8_t borderG = static_cast<uint8_t>(119 + pulse * 45);
    uint8_t borderB = static_cast<uint8_t>(6 + pulse * 20);

    SDL_SetRenderDrawColor(renderer, borderR, borderG, borderB, 255);
    SDL_RenderDrawRect(renderer, &panelBg);
    SDL_Rect panelInner = { panelX + 1, panelY + 1, panelW - 2, panelH - 2 };
    SDL_SetRenderDrawColor(renderer, 45, 55, 75, 255);
    SDL_RenderDrawRect(renderer, &panelInner);

    // 3. Encabezado
    SDL_Rect headerBg = { panelX + 2, panelY + 2, panelW - 4, 75 };
    SDL_SetRenderDrawColor(renderer, 15, 20, 30, 255);
    SDL_RenderFillRect(renderer, &headerBg);
    SDL_SetRenderDrawColor(renderer, 45, 55, 75, 255);
    SDL_RenderDrawLine(renderer, panelX + 2, panelY + 77, panelX + panelW - 2, panelY + 77);

    SDL_Color amberCol = { 245, 158, 11, 255 };
    SDL_Color whiteCol = { 255, 255, 255, 255 };
    SDL_Color mutedCol = { 148, 163, 184, 255 };
    SDL_Color cyanCol  = { 56, 189, 248, 255 };
    SDL_Color greenCol = { 52, 211, 153, 255 };

    drawCenteredText(renderer, "CONKER 64 : RECOMPILED", panelX + panelW / 2, panelY + 16, 3, amberCol, true);
    drawCenteredText(renderer, "NATIVE PC PORT ENGINE  *  64-BIT RECOMPILATION", panelX + panelW / 2, panelY + 48, 1, mutedCol, true);

    // Badge de Versión
    SDL_Rect badge = { panelX + panelW - 130, panelY + 24, 110, 24 };
    SDL_SetRenderDrawColor(renderer, 30, 41, 59, 255);
    SDL_RenderFillRect(renderer, &badge);
    SDL_SetRenderDrawColor(renderer, 56, 189, 248, 255);
    SDL_RenderDrawRect(renderer, &badge);
    drawCenteredText(renderer, "V1.0.0 NATIVE", panelX + panelW - 75, panelY + 32, 1, cyanCol, false);

    // 4. Dropzone Central Interactiva
    int dropX = panelX + 40;
    int dropY = panelY + 95;
    int dropW = panelW - 80;
    int dropH = panelH - 210;

    SDL_Rect dropZone = { dropX, dropY, dropW, dropH };
    if (isHoveringDrop) {
        SDL_SetRenderDrawColor(renderer, 18, 26, 42, 255); // Highlight al pasar el ratón
    } else {
        SDL_SetRenderDrawColor(renderer, 13, 17, 26, 255);
    }
    SDL_RenderFillRect(renderer, &dropZone);

    // Borde animado de la dropzone
    SDL_SetRenderDrawColor(renderer, isHoveringDrop ? 251 : borderR, isHoveringDrop ? 191 : borderG, isHoveringDrop ? 36 : borderB, 255);
    int dash = 12;
    int offset = static_cast<int>(animTime * 22.0f) % (dash * 2);
    for (int x = dropX + offset; x < dropX + dropW; x += dash * 2) {
        SDL_RenderDrawLine(renderer, x, dropY, std::min(x + dash, dropX + dropW), dropY);
        SDL_RenderDrawLine(renderer, x, dropY + dropH, std::min(x + dash, dropX + dropW), dropY + dropH);
    }
    for (int y = dropY + offset; y < dropY + dropH; y += dash * 2) {
        SDL_RenderDrawLine(renderer, dropX, y, dropX, std::min(y + dash, dropY + dropH));
        SDL_RenderDrawLine(renderer, dropX + dropW, y, dropX + dropW, std::min(y + dash, dropY + dropH));
    }

    // Esquinas de diseño
    int corner = 20;
    SDL_SetRenderDrawColor(renderer, 251, 191, 36, 255);
    SDL_RenderDrawLine(renderer, dropX, dropY, dropX + corner, dropY);
    SDL_RenderDrawLine(renderer, dropX, dropY, dropX, dropY + corner);
    SDL_RenderDrawLine(renderer, dropX + dropW - corner, dropY, dropX + dropW, dropY);
    SDL_RenderDrawLine(renderer, dropX + dropW, dropY, dropX + dropW, dropY + corner);
    SDL_RenderDrawLine(renderer, dropX, dropY + dropH - corner, dropX, dropY + dropH);
    SDL_RenderDrawLine(renderer, dropX, dropY + dropH, dropX + corner, dropY + dropH);
    SDL_RenderDrawLine(renderer, dropX + dropW - corner, dropY + dropH, dropX + dropW, dropY + dropH);
    SDL_RenderDrawLine(renderer, dropX + dropW, dropY + dropH - corner, dropX + dropW, dropY + dropH);

    // Icono N64 Detallado en 3D
    int iconCX = dropX + dropW / 2;
    int iconCY = dropY + 55;
    float bounce = std::sin(animTime * 4.0f) * 4.0f;
    int cartY = iconCY + static_cast<int>(bounce);

    // Carcasa del Cartucho N64 con Biseles
    SDL_Rect cartBody = { iconCX - 32, cartY - 22, 64, 46 };
    SDL_SetRenderDrawColor(renderer, 60, 70, 85, 255);
    SDL_RenderFillRect(renderer, &cartBody);
    SDL_SetRenderDrawColor(renderer, 90, 105, 125, 255);
    SDL_RenderDrawRect(renderer, &cartBody);

    // Ranura superior
    SDL_Rect cartNotch = { iconCX - 22, cartY - 22, 44, 4 };
    SDL_SetRenderDrawColor(renderer, 35, 42, 55, 255);
    SDL_RenderFillRect(renderer, &cartNotch);

    // Etiqueta roja auténtica de Conker
    SDL_Rect cartLabel = { iconCX - 24, cartY - 12, 48, 26 };
    SDL_SetRenderDrawColor(renderer, 220, 38, 38, 255);
    SDL_RenderFillRect(renderer, &cartLabel);
    SDL_SetRenderDrawColor(renderer, 245, 158, 11, 255);
    SDL_RenderDrawRect(renderer, &cartLabel);

    // Pines dorados inferiores
    SDL_Rect cartPins = { iconCX - 18, cartY + 24, 36, 4 };
    SDL_SetRenderDrawColor(renderer, 245, 195, 45, 255);
    SDL_RenderFillRect(renderer, &cartPins);

    // Textos de la Dropzone perfectamente espaciados
    drawCenteredText(renderer, "ARRASTRA Y SUELTA TU ARCHIVO ROM AQUI", iconCX, dropY + 115, 2, whiteCol, true);

    // Badge de formatos compatibles
    int pillW = 340, pillH = 26;
    SDL_Rect pillRect = { iconCX - pillW / 2, dropY + 145, pillW, pillH };
    SDL_SetRenderDrawColor(renderer, 24, 34, 52, 255);
    SDL_RenderFillRect(renderer, &pillRect);
    SDL_SetRenderDrawColor(renderer, 56, 189, 248, 255);
    SDL_RenderDrawRect(renderer, &pillRect);
    drawCenteredText(renderer, "FORMATOS: .Z64  |  .N64  |  .V64", iconCX, dropY + 154, 1, cyanCol, false);

    drawCenteredText(renderer, "ROM REQUERIDA: CONKER'S BAD FUR DAY (USA NTSC)", iconCX, dropY + 185, 1, mutedCol, true);
    drawCenteredText(renderer, "TAMBIEN PUEDES HACER CLIC EN ESTE RECUADRO PARA BUSCARLA", iconCX, dropY + 205, 1, greenCol, true);

    // 5. Botón de Acción Principal (Totalmente dimensionado y centrado)
    int btnW = 440;
    int btnH = 50;
    int btnX = panelX + (panelW - btnW) / 2;
    int btnY = panelY + panelH - 95;

    SDL_Rect btnRect = { btnX, btnY, btnW, btnH };
    if (isHoveringBtn) {
        SDL_SetRenderDrawColor(renderer, 245, 158, 11, 255); // Amber-500
    } else {
        SDL_SetRenderDrawColor(renderer, 217, 119, 6, 255);  // Amber-600
    }
    SDL_RenderFillRect(renderer, &btnRect);
    SDL_SetRenderDrawColor(renderer, 251, 191, 36, 255);     // Amber-400
    SDL_RenderDrawRect(renderer, &btnRect);

    // Badge de tecla [ O ]
    SDL_Rect keyBadge = { btnX + 16, btnY + 10, 36, 30 };
    SDL_SetRenderDrawColor(renderer, 15, 23, 42, 255);
    SDL_RenderFillRect(renderer, &keyBadge);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &keyBadge);
    drawCenteredText(renderer, "O", btnX + 34, btnY + 17, 2, whiteCol, false);

    // Texto del botón con alto contraste (Blanco sobre sombra profunda)
    drawText(renderer, "SELECCIONAR ARCHIVO ROM", btnX + 68, btnY + 17, 2, whiteCol, true);

    // 6. Pie de Página / Instrucciones
    int footerY = panelY + panelH - 35;
    drawCenteredText(renderer, "[ O / CLIC ] BUSCAR ARCHIVO       [ ESC ] SALIR", panelX + panelW / 2, footerY, 1, mutedCol, true);

    // 7. Barra de Estado Inferior
    int barY = winH - 28;
    SDL_Rect statusBar = { 0, barY, winW, 28 };
    SDL_SetRenderDrawColor(renderer, 10, 14, 22, 255);
    SDL_RenderFillRect(renderer, &statusBar);
    SDL_SetRenderDrawColor(renderer, 30, 41, 59, 255);
    SDL_RenderDrawLine(renderer, 0, barY, winW, barY);

    SDL_Rect statusDot = { 20, barY + 9, 10, 10 };
    SDL_SetRenderDrawColor(renderer, 245, 158, 11, 255);
    SDL_RenderFillRect(renderer, &statusDot);
    drawText(renderer, "ESTADO: ESPERANDO VOLCADO DE CARTUCHO", 38, barY + 10, 1, mutedCol, false);
    drawText(renderer, "PRESERVACION DIGITAL LIMPIA", winW - 270, barY + 10, 1, mutedCol, false);
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

    bool romLoaded = false;
    std::string loadedRomPath;

    int winW = 1280, winH = 720;
    SDL_GetWindowSize(window, &winW, &winH);

    // 1. Argumento de línea de comandos
    if (argc > 1 && argv[1]) {
        std::string cliPath = argv[1];
        if (std::filesystem::exists(cliPath)) {
            if (startLoadedGame(cliPath, renderer, winW, winH)) {
                romLoaded = true;
                loadedRomPath = cliPath;
            }
        }
    }

    // 2. Búsqueda automática LOCAL
    if (!romLoaded) {
        std::vector<std::string> searchPaths = {
            "baserom.us.z64", "baserom.us.n64", "baserom.us.v64",
            "baserom.z64", "conker.z64", "conker.n64", "conker.v64",
            "Conker's Bad Fur Day (USA).z64", "Conker's Bad Fur Day (USA).n64",
            "Conker's Bad Fur Day.z64"
        };

        for (const auto& path : searchPaths) {
            if (std::filesystem::exists(path)) {
                if (startLoadedGame(path, renderer, winW, winH)) {
                    romLoaded = true;
                    loadedRomPath = path;
                    break;
                }
            }
        }
    }

    if (!romLoaded) {
        std::cout << "[Launcher] Esperando que el usuario arrastre su ROM o presione [O]..." << std::endl;
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
        SDL_GetWindowSize(window, &winW, &winH);

        int mouseX = 0, mouseY = 0;
        SDL_GetMouseState(&mouseX, &mouseY);

        int panelW = std::min(winW - 80, 840);
        int panelH = std::min(winH - 80, 560);
        int panelX = (winW - panelW) / 2;
        int panelY = (winH - panelH) / 2;

        int dropX = panelX + 40;
        int dropY = panelY + 95;
        int dropW = panelW - 80;
        int dropH = panelH - 210;

        int btnW = 440, btnH = 50;
        int btnX = panelX + (panelW - btnW) / 2;
        int btnY = panelY + panelH - 95;

        bool isHoveringDrop = (mouseX >= dropX && mouseX <= dropX + dropW &&
                               mouseY >= dropY && mouseY <= dropY + dropH);
        bool isHoveringBtn  = (mouseX >= btnX && mouseX <= btnX + btnW &&
                               mouseY >= btnY && mouseY <= btnY + btnH);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) isRunning = false;
            else if (event.type == SDL_DROPFILE) {
                char* droppedFile = event.drop.file;
                std::cout << "[Launcher] ROM Arrastrada: " << droppedFile << std::endl;
                if (startLoadedGame(droppedFile, renderer, winW, winH)) {
                    romLoaded = true;
                    loadedRomPath = droppedFile;
                }
                SDL_free(droppedFile);
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (!romLoaded && event.button.button == SDL_BUTTON_LEFT) {
                    if (isHoveringBtn || isHoveringDrop) {
                        std::string selected = openFileDialog();
                        if (!selected.empty() && startLoadedGame(selected, renderer, winW, winH)) {
                            romLoaded = true;
                            loadedRomPath = selected;
                        }
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                else if (event.key.keysym.sym == SDLK_o) {
                    std::string selected = openFileDialog();
                    if (!selected.empty() && startLoadedGame(selected, renderer, winW, winH)) {
                        romLoaded = true;
                        loadedRomPath = selected;
                    }
                }
            }
        }

        const float dt = 1.0f / 60.0f;
        launcherAnimTime += dt;

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

        if (!romLoaded) {
            renderModernLauncher(renderer, winW, winH, launcherAnimTime, isHoveringDrop, isHoveringBtn);
            SDL_RenderPresent(renderer);
            continue;
        }

        N64::InputManager::getInstance().poll(pad);

        float camInputX = 0.0f;
        const Uint8* keyState = SDL_GetKeyboardState(nullptr);
        if (keyState[SDL_SCANCODE_Q] || (pad.button & N64::Buttons::CONT_C)) camInputX -= 1.0f;
        if (keyState[SDL_SCANCODE_E] || (pad.button & N64::Buttons::CONT_F)) camInputX += 1.0f;

        N64::RDPProcessor::getInstance().advanceCameraYaw(camInputX, dt);
        N64::ActorManager::getInstance().updatePlayer(
            pad, dt, N64::RDPProcessor::getInstance().getCameraYaw());

        N64::MIPSRecompiler::getInstance().updateGameLogic(dt);

        const auto& player = N64::ActorManager::getInstance().getPlayer();

        N64::IntroSequence::getInstance().update(dt);

        if (!N64::IntroSequence::getInstance().isGameplayActive()) {
            N64::IntroSequence::getInstance().render(renderer, winW, winH);
        } else {
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
