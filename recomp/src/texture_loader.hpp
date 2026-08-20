#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <cstring>
#include <SDL.h>
#include "memory.hpp"

namespace N64 {

// Formatos oficiales de textura de la Nintendo 64 / RDP
enum class TextureFormat {
    RGBA16 = 0, // 5-5-5-1 (16 bits)
    RGBA32 = 1, // 8-8-8-8 (32 bits)
    IA16   = 2, // Intensity 8-bit + Alpha 8-bit
    IA8    = 3, // Intensity 4-bit + Alpha 4-bit
    CI8    = 4, // Color Indexed 8-bit con paleta TLUT (256 colores)
    CI4    = 5, // Color Indexed 4-bit con paleta TLUT (16 colores)
    I8     = 6  // Intensity 8-bit en escala de grises
};

class TextureLoader {
public:
    static TextureLoader& getInstance() {
        static TextureLoader instance;
        return instance;
    }

    // Decodifica cualquier textura nativa de N64 en RDRAM y crea una textura SDL2 para la GPU
    SDL_Texture* createTextureFromRDRAM(SDL_Renderer* renderer, uint32_t rdramVaddr, int width, int height, TextureFormat fmt, uint32_t tlutVaddr = 0) {
        if (!renderer || width <= 0 || height <= 0) return nullptr;

        uint8_t* rdram = Memory::getInstance().getRDRAM();
        uint32_t paddr = rdramVaddr & 0x1FFFFFFF;

        if (paddr >= Memory::getInstance().getRDRAMSize()) {
            return nullptr;
        }

        std::vector<uint32_t> rgba32Buffer(width * height, 0);

        if (fmt == TextureFormat::RGBA16) {
            decodeRGBA16(&rdram[paddr], rgba32Buffer.data(), width, height);
        } else if (fmt == TextureFormat::RGBA32) {
            decodeRGBA32(&rdram[paddr], rgba32Buffer.data(), width, height);
        } else if (fmt == TextureFormat::IA16) {
            decodeIA16(&rdram[paddr], rgba32Buffer.data(), width, height);
        } else if (fmt == TextureFormat::IA8) {
            decodeIA8(&rdram[paddr], rgba32Buffer.data(), width, height);
        } else if (fmt == TextureFormat::CI8) {
            uint32_t tlutPaddr = tlutVaddr & 0x1FFFFFFF;
            decodeCI8(&rdram[paddr], &rdram[tlutPaddr], rgba32Buffer.data(), width, height);
        } else if (fmt == TextureFormat::CI4) {
            uint32_t tlutPaddr = tlutVaddr & 0x1FFFFFFF;
            decodeCI4(&rdram[paddr], &rdram[tlutPaddr], rgba32Buffer.data(), width, height);
        } else if (fmt == TextureFormat::I8) {
            decodeI8(&rdram[paddr], rgba32Buffer.data(), width, height);
        }

        SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STATIC,
            width,
            height
        );

        if (texture) {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_UpdateTexture(texture, nullptr, rgba32Buffer.data(), width * sizeof(uint32_t));
        }

        return texture;
    }

    // Genera una textura de prueba de Conker (patrón de pelaje/sudadera con logo y cierre)
    SDL_Texture* createConkerProceduralTexture(SDL_Renderer* renderer, int width = 64, int height = 64) {
        if (!renderer) return nullptr;

        std::vector<uint32_t> pixels(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Cuadriculado de N64 en tonos naranja de pelaje de Conker
                bool checker = ((x / 8) + (y / 8)) % 2 == 0;
                uint8_t r = checker ? 240 : 205;
                uint8_t g = checker ? 120 : 95;
                uint8_t b = checker ? 30 : 20;

                // Franja azul central (Sudadera con capucha azul de Conker)
                if (y >= height / 4 && y <= (height * 3) / 4) {
                    r = checker ? 25 : 15;
                    g = checker ? 100 : 75;
                    b = checker ? 230 : 190;
                }

                // Cordones / Cremallera amarilla de Conker
                if (x >= width / 2 - 2 && x <= width / 2 + 2 && y >= height / 4) {
                    r = 255; g = 220; b = 40; // Amarillo brillante
                }

                pixels[y * width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
            }
        }

        SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STATIC,
            width,
            height
        );

        if (texture) {
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_UpdateTexture(texture, nullptr, pixels.data(), width * sizeof(uint32_t));
        }

        return texture;
    }

private:
    TextureLoader() = default;

    // RGBA16: 5 bits Rojo, 5 bits Verde, 5 bits Azul, 1 bit Alfa
    void decodeRGBA16(const uint8_t* src, uint32_t* dst, int w, int h) {
        const uint16_t* s = reinterpret_cast<const uint16_t*>(src);
        for (int i = 0; i < w * h; ++i) {
            uint16_t raw = s[i];
            raw = (raw >> 8) | (raw << 8);

            uint8_t r = (raw >> 11) & 0x1F;
            uint8_t g = (raw >> 6) & 0x1F;
            uint8_t b = (raw >> 1) & 0x1F;
            uint8_t a = (raw & 1) ? 255 : 0;

            r = (r << 3) | (r >> 2);
            g = (g << 3) | (g >> 2);
            b = (b << 3) | (b >> 2);

            dst[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }

    // RGBA32: 8 bits por canal
    void decodeRGBA32(const uint8_t* src, uint32_t* dst, int w, int h) {
        const uint32_t* s = reinterpret_cast<const uint32_t*>(src);
        for (int i = 0; i < w * h; ++i) {
            uint32_t raw = s[i];
            dst[i] = ((raw >> 24) & 0xFF) |
                     ((raw << 8) & 0xFF0000) |
                     ((raw >> 8) & 0xFF00) |
                     ((raw << 24) & 0xFF000000);
        }
    }

    // IA16: 8 bits Intensidad, 8 bits Alfa
    void decodeIA16(const uint8_t* src, uint32_t* dst, int w, int h) {
        for (int i = 0; i < w * h; ++i) {
            uint8_t intensity = src[i * 2];
            uint8_t alpha = src[i * 2 + 1];
            dst[i] = (alpha << 24) | (intensity << 16) | (intensity << 8) | intensity;
        }
    }

    // IA8: 4 bits Intensidad, 4 bits Alfa
    void decodeIA8(const uint8_t* src, uint32_t* dst, int w, int h) {
        for (int i = 0; i < w * h; ++i) {
            uint8_t byte = src[i];
            uint8_t intensity = (byte >> 4) & 0x0F;
            uint8_t alpha = byte & 0x0F;

            intensity = (intensity << 4) | intensity;
            alpha = (alpha << 4) | alpha;

            dst[i] = (alpha << 24) | (intensity << 16) | (intensity << 8) | intensity;
        }
    }

    // CI8: 8-bit index a paleta TLUT (256 colores)
    void decodeCI8(const uint8_t* src, const uint8_t* tlut, uint32_t* dst, int w, int h) {
        const uint16_t* palette = reinterpret_cast<const uint16_t*>(tlut);
        for (int i = 0; i < w * h; ++i) {
            uint8_t idx = src[i];
            uint16_t raw = palette[idx];
            raw = (raw >> 8) | (raw << 8);

            uint8_t r = ((raw >> 11) & 0x1F) << 3;
            uint8_t g = ((raw >> 6) & 0x1F) << 3;
            uint8_t b = ((raw >> 1) & 0x1F) << 3;
            uint8_t a = (raw & 1) ? 255 : 0;

            dst[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }

    // CI4: 4-bit index a paleta TLUT (16 colores)
    void decodeCI4(const uint8_t* src, const uint8_t* tlut, uint32_t* dst, int w, int h) {
        const uint16_t* palette = reinterpret_cast<const uint16_t*>(tlut);
        for (int i = 0; i < (w * h) / 2; ++i) {
            uint8_t byte = src[i];
            uint8_t idx1 = (byte >> 4) & 0x0F;
            uint8_t idx2 = byte & 0x0F;

            for (int k = 0; k < 2; ++k) {
                uint8_t idx = (k == 0) ? idx1 : idx2;
                uint16_t raw = palette[idx];
                raw = (raw >> 8) | (raw << 8);

                uint8_t r = ((raw >> 11) & 0x1F) << 3;
                uint8_t g = ((raw >> 6) & 0x1F) << 3;
                uint8_t b = ((raw >> 1) & 0x1F) << 3;
                uint8_t a = (raw & 1) ? 255 : 0;

                dst[i * 2 + k] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }

    // I8: Escala de grises 8-bit
    void decodeI8(const uint8_t* src, uint32_t* dst, int w, int h) {
        for (int i = 0; i < w * h; ++i) {
            uint8_t val = src[i];
            dst[i] = (0xFF << 24) | (val << 16) | (val << 8) | val;
        }
    }
};

} // namespace N64
