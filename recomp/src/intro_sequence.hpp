#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <SDL.h>
#include "gbi.hpp"
#include "texture_loader.hpp"
#include "audio.hpp"
#include "audio_rom.hpp"

namespace N64 {

enum class IntroStage {
    N64_LOGO_SPIN = 0,    // Logo N64 3D 
    CHAINSAW_CUT = 1,     // Conker cortando el logo con la motosierra
    RAREWARE_GOLD = 2,    // Logo dorado de Rareware "R"
    TITLE_THEME = 3,      // Taberna & Conker en el trono
    GAMEPLAY = 4          // Juego activo
};

class IntroSequence {
public:
    static IntroSequence& getInstance() {
        static IntroSequence instance;
        return instance;
    }

    void init() {
        stage = IntroStage::N64_LOGO_SPIN;
        timer = 0.0f;
        logoAngle = 0.0f;
        logoCutOffset = 0.0f;
        hasPlayedSound = false;
        std::cout << "[Intro] Official Rareware & N64 Intro Cinematic State Machine initialized." << std::endl;
    }

    void update(float dt) {
        timer += dt;

        switch (stage) {
            case IntroStage::N64_LOGO_SPIN:
                logoAngle += 120.0f * dt;
                if (timer > 3.0f) {
                    stage = IntroStage::CHAINSAW_CUT;
                    timer = 0.0f;
                    AudioManager::getInstance().playBootJingle();
                    std::cout << "[Intro] Stage 2: Conker Chainsaw Cut Logo!" << std::endl;
                }
                break;

            case IntroStage::CHAINSAW_CUT:
                logoCutOffset += dt * 1.5f;
                if (timer > 2.5f) {
                    stage = IntroStage::RAREWARE_GOLD;
                    timer = 0.0f;
                    // Musica real del logo de Rareware extraida de la ROM.
                    playROMTrack(MusicTrack::TITLE_RAREWARE, "music_title_rareware");
                    std::cout << "[Intro] Stage 3: Rareware Golden Twinkling R Logo!" << std::endl;
                }
                break;

            case IntroStage::RAREWARE_GOLD:
                if (timer > 3.0f) {
                    stage = IntroStage::TITLE_THEME;
                    timer = 0.0f;
                    playROMTrack(MusicTrack::CONKER_THEME, "music_conker_theme");
                    std::cout << "[Intro] Stage 4: Conker Hungover / Throne Scene!" << std::endl;
                }
                break;

            case IntroStage::TITLE_THEME:
                if (timer > 2.0f) {
                    stage = IntroStage::GAMEPLAY;
                    std::cout << "[Intro] Intro finished -> Transitioning to LIVE GAMEPLAY." << std::endl;
                }
                break;

            case IntroStage::GAMEPLAY:
                break;
        }
    }

    // Renderiza la cinemática oficial de inicio
    void render(SDL_Renderer* renderer, int winW, int winH) {
        if (stage == IntroStage::GAMEPLAY) return;

        // Fondo negro cinemático
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        float cx = winW * 0.5f;
        float cy = winH * 0.5f;

        if (stage == IntroStage::N64_LOGO_SPIN || stage == IntroStage::CHAINSAW_CUT) {
            renderN643DLogo(renderer, cx, cy, logoAngle, logoCutOffset);
        }
        else if (stage == IntroStage::RAREWARE_GOLD) {
            renderRarewareGoldLogo(renderer, cx, cy, timer);
        }
        else if (stage == IntroStage::TITLE_THEME) {
            renderTitleCard(renderer, cx, cy, timer);
        }
    }

    bool isGameplayActive() const { return stage == IntroStage::GAMEPLAY; }
    void skipIntro() { stage = IntroStage::GAMEPLAY; }

private:
    // Reproduce una pista de la ROM; si aun no hay ROM cargada (o el indice no
    // existe en este volcado) recurre al acorde sintetizado en vez de callar.
    static void playROMTrack(size_t index, const char* label) {
        if (index < ROMaudioDecoder::getInstance().trackCount()) {
            std::cout << "[Intro] Playing ROM track " << index << " (" << label << ")" << std::endl;
            ROMaudioPlayer::getInstance().playTrack(index);
        } else {
            std::cout << "[Intro] ROM track " << index << " (" << label
                      << ") unavailable; using synthesized chord." << std::endl;
            AudioManager::getInstance().playBootJingle();
        }
    }

public:

private:
    IntroSequence() : stage(IntroStage::N64_LOGO_SPIN), timer(0.0f), logoAngle(0.0f), logoCutOffset(0.0f), hasPlayedSound(false) {}
    IntroStage stage;
    float timer;
    float logoAngle;
    float logoCutOffset;
    bool hasPlayedSound;

    // Renderiza el Logo N64 en 3D (Rojo, Azul, Verde, Amarillo)
    void renderN643DLogo(SDL_Renderer* renderer, float cx, float cy, float angle, float cutOffset) {
        float rad = angle * 3.14159265f / 180.0f;
        float cosA = std::cos(rad), sinA = std::sin(rad);

        float size = 110.0f;
        float cutLeft = -cutOffset * 60.0f;
        float cutRight = cutOffset * 60.0f;

        // Colores oficiales de N64: Rojo, Verde, Azul, Amarillo
        SDL_Color colors[4] = {
            { 220, 20, 30, 255 },  // Rojo
            { 30, 180, 40, 255 },  // Verde
            { 30, 80, 220, 255 },  // Azul
            { 240, 210, 20, 255 }  // Amarillo
        };

        // Mitad Izquierda del Logo N
        SDL_Rect leftBlock = { static_cast<int>(cx - size + cutLeft), static_cast<int>(cy - size), static_cast<int>(size * 0.45f), static_cast<int>(size * 2.0f) };
        SDL_SetRenderDrawColor(renderer, colors[0].r, colors[0].g, colors[0].b, 255);
        SDL_RenderFillRect(renderer, &leftBlock);

        // Mitad Derecha del Logo N
        SDL_Rect rightBlock = { static_cast<int>(cx + size * 0.55f + cutRight), static_cast<int>(cy - size), static_cast<int>(size * 0.45f), static_cast<int>(size * 2.0f) };
        SDL_SetRenderDrawColor(renderer, colors[2].r, colors[2].g, colors[2].b, 255);
        SDL_RenderFillRect(renderer, &rightBlock);

        // Barra Diagonal
        SDL_Rect diagBlock = { static_cast<int>(cx - size * 0.4f), static_cast<int>(cy - size * 0.8f), static_cast<int>(size * 0.8f), static_cast<int>(size * 1.6f) };
        SDL_SetRenderDrawColor(renderer, colors[3].r, colors[3].g, colors[3].b, 255);
        SDL_RenderFillRect(renderer, &diagBlock);

        if (stage == IntroStage::CHAINSAW_CUT) {
            // Destello de chispas de motosierra
            SDL_SetRenderDrawColor(renderer, 255, 240, 100, 255);
            SDL_RenderDrawLine(renderer, static_cast<int>(cx - 80), static_cast<int>(cy), static_cast<int>(cx + 80), static_cast<int>(cy));
        }
    }

    // Renderiza el icónico logo dorado "R" de Rareware con brillo
    void renderRarewareGoldLogo(SDL_Renderer* renderer, float cx, float cy, float t) {
        float scale = (t * 1.5f > 1.0f) ? 1.0f : t * 1.5f;
        float w = 180.0f * scale;
        float h = 180.0f * scale;

        // Borde dorado brillante de Rareware
        SDL_Rect goldPlate = { static_cast<int>(cx - w * 0.5f), static_cast<int>(cy - h * 0.5f), static_cast<int>(w), static_cast<int>(h) };
        SDL_SetRenderDrawColor(renderer, 218, 165, 32, 255); // Dorado N64
        SDL_RenderFillRect(renderer, &goldPlate);

        // Letra "R" estilizada
        SDL_Rect rInner = { static_cast<int>(cx - w * 0.35f), static_cast<int>(cy - h * 0.35f), static_cast<int>(w * 0.7f), static_cast<int>(h * 0.7f) };
        SDL_SetRenderDrawColor(renderer, 40, 20, 10, 255);
        SDL_RenderFillRect(renderer, &rInner);

        // Brillo "Twinkle"
        int twinkle = static_cast<int>(std::sin(t * 10.0f) * 127 + 128);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(twinkle));
        SDL_RenderDrawLine(renderer, static_cast<int>(cx - w * 0.5f), static_cast<int>(cy - h * 0.5f), static_cast<int>(cx - w * 0.5f + 40), static_cast<int>(cy - h * 0.5f + 40));
    }

    // Pantalla de título oficial de Conker
    void renderTitleCard(SDL_Renderer* renderer, float cx, float cy, float t) {
        (void)t;
        SDL_Rect banner = { static_cast<int>(cx - 240), static_cast<int>(cy - 60), 480, 120 };
        SDL_SetRenderDrawColor(renderer, 200, 40, 20, 255); // Rojo Conker
        SDL_RenderFillRect(renderer, &banner);

        SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
        SDL_RenderDrawRect(renderer, &banner);
    }
};

} // namespace N64
