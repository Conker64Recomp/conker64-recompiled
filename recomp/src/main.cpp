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
// Proporciona tipografía limpia y nítida a cualquier escala sin dependencias externas.
static const uint8_t font8x8_basic[128][8] = {
    // 0-31 control chars (empty)
    {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
    {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // ! (33)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},
    // " (34)
    {0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00},
    // # (35)
    {0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00},
    // $ (36)
    {0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00},
    // % (37)
    {0x00, 0x63, 0x66, 0x0C, 0x18, 0x33, 0x63, 0x00},
    // & (38)
    {0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00},
    // ' (39)
    {0x18, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00},
    // ( (40)
    {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00},
    // ) (41)
    {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00},
    // * (42)
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},
    // + (43)
    {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00},
    // , (44)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x08},
    // - (45)
    {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    // . (46)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
    // / (47)
    {0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00},
    // 0-9 (48-57)
    {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    {0x3C, 0x66, 0x06, 0x1C, 0x30, 0x66, 0x7E, 0x00},
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    {0x0C, 0x1C, 0x3C, 0x6C, 0xFE, 0x0C, 0x0C, 0x00},
    {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
    {0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00},
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
    {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00},
    // : (58)
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00},
    // ; (59)
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x08},
    // < (60)
    {0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00},
    // = (61)
    {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00},
    // > (62)
    {0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00},
    // ? (63)
    {0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00},
    // @ (64)
    {0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x62, 0x3C, 0x00},
    // A-Z (65-90)
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
    // [ (91)
    {0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00},
    // \ (92)
    {0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00},
    // ] (93)
    {0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00},
    // ^ (94)
    {0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
    // _ (95)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},
    // ` (96)
    {0x18, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00},
    // a-z (97-122) - Rendered uppercase for maximum arcade clarity
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
    // { | } ~
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
        SDL_Color sc = { 0, 0, 0, 180 };
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

// ─── MODERN TAILWIND-INSPIRED LAUNCHER UI ───────────────────────────────────
void renderModernLauncher(SDL_Renderer* renderer, int winW, int winH, float animTime, bool isHoveringBtn) {
    // 1. Fondo Slate-950 con degradado cinematográfico
    for (int y = 0; y < winH; ++y) {
        float t = static_cast<float>(y) / static_cast<float>(winH);
        uint8_t bgR = static_cast<uint8_t>(11 + t * 8);   // #0B0F17 -> #131B2A
        uint8_t bgG = static_cast<uint8_t>(15 + t * 12);
        uint8_t bgB = static_cast<uint8_t>(23 + t * 18);
        SDL_SetRenderDrawColor(renderer, bgR, bgG, bgB, 255);
        SDL_RenderDrawLine(renderer, 0, y, winW, y);
    }

    // Grid sutil con puntos de acento (estilo Tailwind/Modern UI)
    SDL_SetRenderDrawColor(renderer, 30, 41, 59, 100);
    int gridStep = 48;
    for (int x = 0; x < winW; x += gridStep) {
        for (int y = 0; y < winH; y += gridStep) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    // 2. Tarjeta Principal Glassmorphism (Slate-900 con borde iluminado)
    int panelW = std::min(winW - 80, 840);
    int panelH = std::min(winH - 80, 540);
    int panelX = (winW - panelW) / 2;
    int panelY = (winH - panelH) / 2;

    // Sombra profunda multicapa
    SDL_Rect shadow2 = { panelX + 10, panelY + 10, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 2, 6, 12, 180);
    SDL_RenderFillRect(renderer, &shadow2);

    SDL_Rect shadow1 = { panelX + 4, panelY + 4, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 5, 10, 20, 240);
    SDL_RenderFillRect(renderer, &shadow1);

    // Fondo del panel principal (Zinc/Slate-900)
    SDL_Rect panelBg = { panelX, panelY, panelW, panelH };
    SDL_SetRenderDrawColor(renderer, 22, 27, 38, 255);
    SDL_RenderFillRect(renderer, &panelBg);

    // Borde exterior con pulso sutil ámbar Conker
    float pulse = (std::sin(animTime * 3.0f) + 1.0f) * 0.5f;
    uint8_t borderR = static_cast<uint8_t>(217 + pulse * 38); // Amber-500 (#F59E0B)
    uint8_t borderG = static_cast<uint8_t>(119 + pulse * 45);
    uint8_t borderB = static_cast<uint8_t>(6 + pulse * 20);
    SDL_SetRenderDrawColor(renderer, borderR, borderG, borderB, 255);
    SDL_RenderDrawRect(renderer, &panelBg);

    // Borde interior sutil
    SDL_Rect panelInner = { panelX + 1, panelY + 1, panelW - 2, panelH - 2 };
    SDL_SetRenderDrawColor(renderer, 51, 65, 85, 255);
    SDL_RenderDrawRect(renderer, &panelInner);

    // 3. Encabezado de la Tarjeta
    SDL_Rect headerBg = { panelX + 2, panelY + 2, panelW - 4, 75 };
    SDL_SetRenderDrawColor(renderer, 15, 20, 30, 255);
    SDL_RenderFillRect(renderer, &headerBg);
    SDL_SetRenderDrawColor(renderer, 51, 65, 85, 255);
    SDL_RenderDrawLine(renderer, panelX + 2, panelY + 77, panelX + panelW - 2, panelY + 77);

    // Título Principal
    SDL_Color amberCol = { 245, 158, 11, 255 };  // Tailwind Amber-500
    SDL_Color whiteCol = { 255, 255, 255, 255 };
    SDL_Color mutedCol = { 148, 163, 184, 255 };  // Tailwind Slate-400
    SDL_Color cyanCol  = { 56, 189, 248, 255 };   // Tailwind Sky-400
    SDL_Color greenCol = { 52, 211, 153, 255 };   // Tailwind Emerald-400

    drawCenteredText(renderer, "CONKER 64 : RECOMPILED", panelX + panelW / 2, panelY + 16, 3, amberCol, true);
    drawCenteredText(renderer, "NATIVE PC PORT ENGINE  *  X86_64 ARCHITECTURE", panelX + panelW / 2, panelY + 48, 1, mutedCol, true);

    // Badge de Versión
    SDL_Rect badge = { panelX + panelW - 130, panelY + 24, 110, 22 };
    SDL_SetRenderDrawColor(renderer, 30, 41, 59, 255);
    SDL_RenderFillRect(renderer, &badge);
    SDL_SetRenderDrawColor(renderer, 56, 189, 248, 255);
    SDL_RenderDrawRect(renderer, &badge);
    drawCenteredText(renderer, "V1.0.0 NATIVE", panelX + panelW - 75, panelY + 31, 1, cyanCol, false);

    // 4. Zona de Drop Central (Dropzone Interactiva)
    int dropX = panelX + 40;
    int dropY = panelY + 95;
    int dropW = panelW - 80;
    int dropH = panelH - 210;

    SDL_Rect dropZone = { dropX, dropY, dropW, dropH };
    SDL_SetRenderDrawColor(renderer, 13, 17, 26, 255);
    SDL_RenderFillRect(renderer, &dropZone);

    // Borde animado discontinuo en la Dropzone
    SDL_SetRenderDrawColor(renderer, borderR, borderG, borderB, 200);
    int dash = 12;
    int offset = static_cast<int>(animTime * 20.0f) % (dash * 2);
    for (int x = dropX + offset; x < dropX + dropW; x += dash * 2) {
        SDL_RenderDrawLine(renderer, x, dropY, std::min(x + dash, dropX + dropW), dropY);
        SDL_RenderDrawLine(renderer, x, dropY + dropH, std::min(x + dash, dropX + dropW), dropY + dropH);
    }
    for (int y = dropY + offset; y < dropY + dropH; y += dash * 2) {
        SDL_RenderDrawLine(renderer, dropX, y, dropX, std::min(y + dash, dropY + dropH));
        SDL_RenderDrawLine(renderer, dropX + dropW, y, dropX + dropW, std::min(y + dash, dropY + dropH));
    }

    // Corchetes angulares en las 4 esquinas de la Dropzone
    int corner = 18;
    SDL_SetRenderDrawColor(renderer, 251, 191, 36, 255);
    SDL_RenderDrawLine(renderer, dropX, dropY, dropX + corner, dropY);
    SDL_RenderDrawLine(renderer, dropX, dropY, dropX, dropY + corner);
    SDL_RenderDrawLine(renderer, dropX + dropW - corner, dropY, dropX + dropW, dropY);
    SDL_RenderDrawLine(renderer, dropX + dropW, dropY, dropX + dropW, dropY + corner);
    SDL_RenderDrawLine(renderer, dropX, dropY + dropH - corner, dropX, dropY + dropH);
    SDL_RenderDrawLine(renderer, dropX, dropY + dropH, dropX + corner, dropY + dropH);
    SDL_RenderDrawLine(renderer, dropX + dropW - corner, dropY + dropH, dropX + dropW, dropY + dropH);
    SDL_RenderDrawLine(renderer, dropX + dropW, dropY + dropH - corner, dropX + dropW, dropY + dropH);

    // Icono N64 Central Flotante
    int iconCX = dropX + dropW / 2;
    int iconCY = dropY + 60;
    float bounce = std::sin(animTime * 4.0f) * 5.0f;
    int cartY = iconCY + static_cast<int>(bounce);

    SDL_Rect cart = { iconCX - 24, cartY - 18, 48, 36 };
    SDL_SetRenderDrawColor(renderer, 51, 65, 85, 255);
    SDL_RenderFillRect(renderer, &cart);
    SDL_SetRenderDrawColor(renderer, borderR, borderG, borderB, 255);
    SDL_RenderDrawRect(renderer, &cart);

    SDL_Rect cartLabel = { iconCX - 16, cartY - 10, 32, 20 };
    SDL_SetRenderDrawColor(renderer, 220, 38, 38, 255); // Red-600
    SDL_RenderFillRect(renderer, &cartLabel);

    // Texto Central dentro de la Dropzone
    drawCenteredText(renderer, "ARRASTRA Y SUELTA TU ARCHIVO ROM AQUI", iconCX, dropY + 115, 2, whiteCol, true);
    drawCenteredText(renderer, "FORMATOS COMPATIBLES: .Z64  |  .N64  |  .V64", iconCX, dropY + 145, 1, cyanCol, true);
    drawCenteredText(renderer, "ROM REQUERIDA: CONKER'S BAD FUR DAY (USA NTSC)", iconCX, dropY + 165, 1, mutedCol, true);

    // 5. Botón de Acción Principal (Tailwind Amber Button)
    int btnW = 380;
    int btnH = 48;
    int btnX = panelX + (panelW - btnW) / 2;
    int btnY = panelY + panelH - 100;

    SDL_Rect btnRect = { btnX, btnY, btnW, btnH };
    if (isHoveringBtn) {
        SDL_SetRenderDrawColor(renderer, 245, 158, 11, 255); // Amber-500
    } else {
        SDL_SetRenderDrawColor(renderer, 217, 119, 6, 255);  // Amber-600
    }
    SDL_RenderFillRect(renderer, &btnRect);

    SDL_SetRenderDrawColor(renderer, 251, 191, 36, 255);     // Amber-400
    SDL_RenderDrawRect(renderer, &btnRect);

    // Badge [ O ] dentro del botón
    SDL_Rect keyBadge = { btnX + 16, btnY + 10, 32, 28 };
    SDL_SetRenderDrawColor(renderer, 15, 23, 42, 255);
    SDL_RenderFillRect(renderer, &keyBadge);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &keyBadge);
    drawCenteredText(renderer, "O", btnX + 32, btnY + 16, 2, whiteCol, false);

    drawText(renderer, "BUSCAR ROM EN EL EQUIPO", btnX + 60, btnY + 17, 2, { 15, 23, 42, 255 }, false);

    // 6. Pie de Página / Instrucciones de Atajos
    int footerY = panelY + panelH - 38;
    drawCenteredText(renderer, "[ O ] SELECCIONAR ARCHIVO       [ ESC ] SALIR DEL JUEGO", panelX + panelW / 2, footerY, 1, mutedCol, true);

    // Barra de Estado Inferior
    int barY = winH - 28;
    SDL_Rect statusBar = { 0, barY, winW, 28 };
    SDL_SetRenderDrawColor(renderer, 10, 14, 22, 255);
    SDL_RenderFillRect(renderer, &statusBar);
    SDL_SetRenderDrawColor(renderer, 30, 41, 59, 255);
    SDL_RenderDrawLine(renderer, 0, barY, winW, barY);

    // Indicador de Estado (Luz verde/amarilla)
    SDL_Rect statusDot = { 20, barY + 9, 10, 10 };
    SDL_SetRenderDrawColor(renderer, 245, 158, 11, 255);
    SDL_RenderFillRect(renderer, &statusDot);
    drawText(renderer, "ESTADO: ESPERANDO VOLCADO DE CARTUCHO", 38, barY + 10, 1, mutedCol, false);
    drawText(renderer, "PRESERVACION DIGITAL SIN COPYRIGHT", winW - 320, barY + 10, 1, mutedCol, false);
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

    // Comprobación de ROM
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
                if (startLoadedGame(path, renderer)) {
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
    bool isHoveringButton = false;
    N64::OSContPad pad{};

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        int mouseX = 0, mouseY = 0;
        SDL_GetMouseState(&mouseX, &mouseY);

        int winW = 1280, winH = 720;
        SDL_GetWindowSize(window, &winW, &winH);

        int panelW = std::min(winW - 80, 840);
        int panelH = std::min(winH - 80, 540);
        int panelX = (winW - panelW) / 2;
        int panelY = (winH - panelH) / 2;
        int btnW = 380, btnH = 48;
        int btnX = panelX + (panelW - btnW) / 2;
        int btnY = panelY + panelH - 100;

        isHoveringButton = (mouseX >= btnX && mouseX <= btnX + btnW &&
                            mouseY >= btnY && mouseY <= btnY + btnH);

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
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (!romLoaded && event.button.button == SDL_BUTTON_LEFT && isHoveringButton) {
                    std::string selected = openFileDialog();
                    if (!selected.empty() && startLoadedGame(selected, renderer)) {
                        romLoaded = true;
                        loadedRomPath = selected;
                    }
                }
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
        // CONTROL DE FLUJO: SI NO HAY ROM CARGADA, RENDERIZAR LAUNCHER SIN LAG
        // =========================================================================
        if (!romLoaded) {
            renderModernLauncher(renderer, winW, winH, launcherAnimTime, isHoveringButton);
            SDL_RenderPresent(renderer);
            // Cero delays artificiales: SDL_RENDERER_PRESENTVSYNC maneja la tasa de refresco a 60/120+ FPS sin lag
            continue;
        }

        // =========================================================================
        // FLUJO DE JUEGO ACTIVO (SOLO CON ROM VALIDA)
        // =========================================================================
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
