#pragma once

#include <cstdint>
#include <vector>
#include <SDL.h>
#include "memory.hpp"

namespace N64 {

enum class VideoMode {
    NTSC_320x240,
    PAL_320x288,
    MPAL_320x240,
    FPAL_320x288
};

class VideoInterface {
public:
    VideoInterface() : width(320), height(240), origin(0), format(0) {}

    void setOrigin(uint32_t vaddr) { origin = vaddr; }
    void setWidth(int w) { width = w; }
    void setHeight(int h) { height = h; }
    void setFormat(int fmt) { format = fmt; } // 2 = RGBA16 (5551), 3 = RGBA32 (8888)

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Converts N64 framebuffer from RDRAM to an RGBA32 texture for SDL2
    void updateTexture(SDL_Texture* targetTexture) {
        if (!targetTexture) return;

        uint8_t* rdram = Memory::getInstance().getRDRAM();
        if (!rdram) return;

        uint32_t paddr = origin & 0x1FFFFFFF;
        if (paddr >= RDRAM_SIZE) return;

        void* pixels = nullptr;
        int pitch = 0;
        if (SDL_LockTexture(targetTexture, nullptr, &pixels, &pitch) != 0) {
            return;
        }

        uint32_t* outPixels = static_cast<uint32_t*>(pixels);

        if (format == 2 || format == 0) {
            // 16-bit RGBA 5551 (N64 standard format used by most games including Conker)
            uint16_t* inPixels = reinterpret_cast<uint16_t*>(&rdram[paddr]);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    uint16_t raw = inPixels[y * width + x];
                    // Big-endian swap
                    raw = (raw >> 8) | (raw << 8);

                    uint8_t r = (raw >> 11) & 0x1F;
                    uint8_t g = (raw >> 6) & 0x1F;
                    uint8_t b = (raw >> 1) & 0x1F;
                    uint8_t a = (raw & 1) ? 255 : 0;

                    // Expand 5-bit to 8-bit
                    r = (r << 3) | (r >> 2);
                    g = (g << 3) | (g >> 2);
                    b = (b << 3) | (b >> 2);

                    outPixels[y * (pitch / 4) + x] = (a << 24) | (r << 16) | (g << 8) | b;
                }
            }
        } else if (format == 3) {
            // 32-bit RGBA 8888
            uint32_t* inPixels = reinterpret_cast<uint32_t*>(&rdram[paddr]);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    uint32_t raw = inPixels[y * width + x];
                    // Swap endian
                    raw = ((raw >> 24) & 0xff) | ((raw << 8) & 0xff0000) | ((raw >> 8) & 0xff00) | ((raw << 24) & 0xff000000);
                    outPixels[y * (pitch / 4) + x] = raw;
                }
            }
        }

        SDL_UnlockTexture(targetTexture);
    }

private:
    int width;
    int height;
    uint32_t origin;
    int format;
};

} // namespace N64
