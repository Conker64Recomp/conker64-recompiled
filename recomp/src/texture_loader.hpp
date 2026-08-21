#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <cstring>
#include <string>
#include <SDL.h>
#include "memory.hpp"

// stb_image — single-header PNG/JPG/BMP loader (MIT licence)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

namespace N64 {

enum class TextureFormat {
    RGBA16 = 0, RGBA32 = 1, IA16 = 2, IA8 = 3,
    CI8 = 4, CI4 = 5, I8 = 6
};

class TextureLoader {
public:
    static TextureLoader& getInstance() {
        static TextureLoader instance;
        return instance;
    }

    // ── Load a PNG file from disk using stb_image ─────────────────────────
    SDL_Texture* loadPNG(SDL_Renderer* renderer, const std::string& path) {
        if (!renderer || path.empty()) return nullptr;

        // Try several base paths
        const char* bases[] = {
            "",
            "../",
            "../../",
            "exported_assets/",
            nullptr
        };

        std::string fullPath;
        FILE* fp = nullptr;
        for (int i = 0; bases[i]; ++i) {
            fullPath = std::string(bases[i]) + path;
            fp = fopen(fullPath.c_str(), "rb");
            if (fp) break;
        }
        if (!fp) {
            fp = fopen(path.c_str(), "rb");
            if (!fp) return nullptr;
        }

        // Read file into buffer
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        std::vector<uint8_t> buf(fsize);
        fread(buf.data(), 1, fsize, fp);
        fclose(fp);

        int w, h, ch;
        uint8_t* pixels = stbi_load_from_memory(buf.data(), (int)buf.size(), &w, &h, &ch, 4);
        if (!pixels) return nullptr;

        SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                             SDL_TEXTUREACCESS_STATIC, w, h);
        if (tex) {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_UpdateTexture(tex, nullptr, pixels, w * 4);
        }
        stbi_image_free(pixels);
        return tex;
    }

    // ── Decode any native N64 texture format from RDRAM ──────────────────
    SDL_Texture* createTextureFromRDRAM(SDL_Renderer* renderer,
                                        uint32_t rdramVaddr, int width, int height,
                                        TextureFormat fmt, uint32_t tlutVaddr = 0) {
        if (!renderer || width <= 0 || height <= 0) return nullptr;
        uint8_t* rdram = Memory::getInstance().getRDRAM();
        uint32_t paddr = rdramVaddr & 0x1FFFFFFF;
        if (paddr >= Memory::getInstance().getRDRAMSize()) return nullptr;
        std::vector<uint32_t> buf(width * height, 0);
        if      (fmt == TextureFormat::RGBA16) decodeRGBA16(&rdram[paddr], buf.data(), width, height);
        else if (fmt == TextureFormat::RGBA32) decodeRGBA32(&rdram[paddr], buf.data(), width, height);
        else if (fmt == TextureFormat::IA16)   decodeIA16  (&rdram[paddr], buf.data(), width, height);
        else if (fmt == TextureFormat::IA8)    decodeIA8   (&rdram[paddr], buf.data(), width, height);
        else if (fmt == TextureFormat::CI8) {
            uint32_t tp = tlutVaddr & 0x1FFFFFFF;
            decodeCI8(&rdram[paddr], &rdram[tp], buf.data(), width, height);
        }
        else if (fmt == TextureFormat::CI4) {
            uint32_t tp = tlutVaddr & 0x1FFFFFFF;
            decodeCI4(&rdram[paddr], &rdram[tp], buf.data(), width, height);
        }
        else if (fmt == TextureFormat::I8)     decodeI8    (&rdram[paddr], buf.data(), width, height);
        SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STATIC, width, height);
        if (tex) {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_UpdateTexture(tex, nullptr, buf.data(), width * sizeof(uint32_t));
        }
        return tex;
    }

    // ── Procedural fallback texture (Conker fur pattern) ─────────────────
    SDL_Texture* createConkerProceduralTexture(SDL_Renderer* renderer,
                                               int width = 64, int height = 64) {
        if (!renderer) return nullptr;
        std::vector<uint32_t> pixels(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                bool checker = ((x / 8) + (y / 8)) % 2 == 0;
                uint8_t r = checker ? 240 : 205;
                uint8_t g = checker ? 120 : 95;
                uint8_t b = checker ? 30  : 20;
                if (y >= height/4 && y <= (height*3)/4) {
                    r = checker ? 25 : 15;
                    g = checker ? 100 : 75;
                    b = checker ? 230 : 190;
                }
                if (x >= width/2-2 && x <= width/2+2 && y >= height/4) {
                    r = 255; g = 220; b = 40;
                }
                pixels[y*width+x] = (0xFF<<24)|(r<<16)|(g<<8)|b;
            }
        }
        SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STATIC, width, height);
        if (tex) {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_UpdateTexture(tex, nullptr, pixels.data(), width * sizeof(uint32_t));
        }
        return tex;
    }

private:
    TextureLoader() = default;

    void decodeRGBA16(const uint8_t* src, uint32_t* dst, int w, int h) {
        const uint16_t* s = reinterpret_cast<const uint16_t*>(src);
        for (int i = 0; i < w*h; ++i) {
            uint16_t raw = (s[i] >> 8) | (s[i] << 8);
            uint8_t r = ((raw>>11)&0x1F)<<3; r |= r>>5;
            uint8_t g = ((raw>>6) &0x1F)<<3; g |= g>>5;
            uint8_t b = ((raw>>1) &0x1F)<<3; b |= b>>5;
            uint8_t a = (raw&1) ? 255 : 0;
            dst[i] = (a<<24)|(r<<16)|(g<<8)|b;
        }
    }
    void decodeRGBA32(const uint8_t* src, uint32_t* dst, int w, int h) {
        const uint32_t* s = reinterpret_cast<const uint32_t*>(src);
        for (int i = 0; i < w*h; ++i) {
            uint32_t raw = s[i];
            dst[i] = ((raw>>24)&0xFF)|((raw<<8)&0xFF0000)|((raw>>8)&0xFF00)|((raw<<24)&0xFF000000);
        }
    }
    void decodeIA16(const uint8_t* src, uint32_t* dst, int w, int h) {
        for (int i = 0; i < w*h; ++i) {
            uint8_t I = src[i*2], A = src[i*2+1];
            dst[i] = (A<<24)|(I<<16)|(I<<8)|I;
        }
    }
    void decodeIA8(const uint8_t* src, uint32_t* dst, int w, int h) {
        for (int i = 0; i < w*h; ++i) {
            uint8_t byte = src[i];
            uint8_t I = ((byte>>4)&0xF); I = (I<<4)|I;
            uint8_t A = (byte&0xF);       A = (A<<4)|A;
            dst[i] = (A<<24)|(I<<16)|(I<<8)|I;
        }
    }
    void decodeCI8(const uint8_t* src, const uint8_t* tlut, uint32_t* dst, int w, int h) {
        const uint16_t* pal = reinterpret_cast<const uint16_t*>(tlut);
        for (int i = 0; i < w*h; ++i) {
            uint16_t raw = (pal[src[i]]>>8)|(pal[src[i]]<<8);
            uint8_t r = ((raw>>11)&0x1F)<<3;
            uint8_t g = ((raw>>6) &0x1F)<<3;
            uint8_t b = ((raw>>1) &0x1F)<<3;
            uint8_t a = (raw&1)?255:0;
            dst[i] = (a<<24)|(r<<16)|(g<<8)|b;
        }
    }
    void decodeCI4(const uint8_t* src, const uint8_t* tlut, uint32_t* dst, int w, int h) {
        const uint16_t* pal = reinterpret_cast<const uint16_t*>(tlut);
        for (int i = 0; i < (w*h)/2; ++i) {
            for (int k = 0; k < 2; ++k) {
                uint8_t idx = (k==0)?(src[i]>>4):(src[i]&0xF);
                uint16_t raw = (pal[idx]>>8)|(pal[idx]<<8);
                uint8_t r = ((raw>>11)&0x1F)<<3;
                uint8_t g = ((raw>>6) &0x1F)<<3;
                uint8_t b = ((raw>>1) &0x1F)<<3;
                uint8_t a = (raw&1)?255:0;
                dst[i*2+k] = (a<<24)|(r<<16)|(g<<8)|b;
            }
        }
    }
    void decodeI8(const uint8_t* src, uint32_t* dst, int w, int h) {
        for (int i = 0; i < w*h; ++i) {
            uint8_t v = src[i];
            dst[i] = (0xFF<<24)|(v<<16)|(v<<8)|v;
        }
    }
};

} // namespace N64
