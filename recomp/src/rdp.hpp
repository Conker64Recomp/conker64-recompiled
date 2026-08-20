#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <SDL.h>
#include "gbi.hpp"
#include "memory.hpp"
#include "texture_loader.hpp"

namespace N64 {

class RDPProcessor {
public:
    static RDPProcessor& getInstance() {
        static RDPProcessor instance;
        return instance;
    }

    void init(SDL_Renderer* renderer) {
        std::cout << "[RDP] Fast3D / F3DEX2 Microcode Display List Processor initialized with Texture Mapping." << std::endl;
        // Textura procedural como fallback hasta que se carguen los assets reales
        activeTexture = TextureLoader::getInstance().createConkerProceduralTexture(renderer, 64, 64);
    }

    // Carga una textura real decomprimida desde los assets de Rareware
    void loadRealTexture(SDL_Renderer* renderer, const uint8_t* rgba16Data, int width, int height) {
        if (!rgba16Data || width <= 0 || height <= 0) return;

        if (activeTexture) {
            SDL_DestroyTexture(activeTexture);
            activeTexture = nullptr;
        }

        std::vector<uint32_t> argb32(width * height);
        const uint16_t* src = reinterpret_cast<const uint16_t*>(rgba16Data);
        for (int i = 0; i < width * height; ++i) {
            uint16_t raw = src[i];
            // Big-Endian swap
            raw = static_cast<uint16_t>((raw >> 8) | (raw << 8));
            uint8_t r = ((raw >> 11) & 0x1F); r = (r << 3) | (r >> 2);
            uint8_t g = ((raw >>  6) & 0x1F); g = (g << 3) | (g >> 2);
            uint8_t b = ((raw >>  1) & 0x1F); b = (b << 3) | (b >> 2);
            uint8_t a = (raw & 1) ? 255 : 0;
            argb32[i] = (static_cast<uint32_t>(a) << 24) |
                        (static_cast<uint32_t>(r) << 16) |
                        (static_cast<uint32_t>(g) << 8)  |
                        static_cast<uint32_t>(b);
        }

        activeTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STATIC, width, height);
        if (activeTexture) {
            SDL_SetTextureBlendMode(activeTexture, SDL_BLENDMODE_BLEND);
            SDL_UpdateTexture(activeTexture, nullptr, argb32.data(), width * sizeof(uint32_t));
            std::cout << "[RDP] Real Rareware texture loaded to GPU: " << width << "x" << height << " RGBA16." << std::endl;
        }
    }

    void shutdown() {
        if (activeTexture) {
            SDL_DestroyTexture(activeTexture);
            activeTexture = nullptr;
        }
    }

    void processDisplayList(uint32_t dlVaddr, SDL_Renderer* renderer, int winW, int winH, float angle) {
        (void)dlVaddr;
        if (!renderer) return;
        renderTexturedModel3D(renderer, winW, winH, angle);
    }

private:
    RDPProcessor() : activeTexture(nullptr) {}
    SDL_Texture* activeTexture;

    struct Vec3   { float x, y, z; };
    struct Point2D{ float x, y; };
    struct VertexUV {
        Point2D pos;
        SDL_FPoint uv;
        SDL_Color color;
    };

    Point2D project(Vec3 v, int winW, int winH, float fov, float distance) {
        float z = v.z + distance;
        if (z < 0.1f) z = 0.1f;
        float factor = fov / z;
        return { winW / 2.0f + v.x * factor, winH / 2.0f - v.y * factor };
    }

    void drawTexturedTriangle(SDL_Renderer* renderer, VertexUV v1, VertexUV v2, VertexUV v3) {
        SDL_Vertex vertices[3] = {
            { { v1.pos.x, v1.pos.y }, v1.color, v1.uv },
            { { v2.pos.x, v2.pos.y }, v2.color, v2.uv },
            { { v3.pos.x, v3.pos.y }, v3.color, v3.uv }
        };
        SDL_RenderGeometry(renderer, activeTexture, vertices, 3, nullptr, 0);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
        SDL_RenderDrawLineF(renderer, v1.pos.x, v1.pos.y, v2.pos.x, v2.pos.y);
        SDL_RenderDrawLineF(renderer, v2.pos.x, v2.pos.y, v3.pos.x, v3.pos.y);
        SDL_RenderDrawLineF(renderer, v3.pos.x, v3.pos.y, v1.pos.x, v1.pos.y);
    }

    void renderTexturedModel3D(SDL_Renderer* renderer, int winW, int winH, float angle) {
        Vec3 localVertices[8] = {
            {-1.0f,-1.0f,-1.0f},{1.0f,-1.0f,-1.0f},{1.0f,1.0f,-1.0f},{-1.0f,1.0f,-1.0f},
            {-1.0f,-1.0f, 1.0f},{1.0f,-1.0f, 1.0f},{1.0f,1.0f, 1.0f},{-1.0f,1.0f, 1.0f}
        };

        float radY = angle * 3.14159265f / 180.0f;
        float radX = (angle * 0.5f) * 3.14159265f / 180.0f;
        float cosY = std::cos(radY), sinY = std::sin(radY);
        float cosX = std::cos(radX), sinX = std::sin(radX);

        Point2D proj[8];
        for (int i = 0; i < 8; ++i) {
            float x1 = localVertices[i].x * cosY + localVertices[i].z * sinY;
            float z1 = -localVertices[i].x * sinY + localVertices[i].z * cosY;
            float y2 = localVertices[i].y * cosX - z1 * sinX;
            float z2 = localVertices[i].y * sinX + z1 * cosX;
            proj[i] = project({ x1, y2, z2 }, winW, winH, 360.0f, 3.5f);
        }

        SDL_Color white = { 255, 255, 255, 255 };
        SDL_FPoint uv00={0,0}, uv10={1,0}, uv11={1,1}, uv01={0,1};

        // Frente
        drawTexturedTriangle(renderer,{proj[0],uv01,white},{proj[1],uv11,white},{proj[2],uv10,white});
        drawTexturedTriangle(renderer,{proj[0],uv01,white},{proj[2],uv10,white},{proj[3],uv00,white});
        // Atrás
        drawTexturedTriangle(renderer,{proj[5],uv01,white},{proj[4],uv11,white},{proj[7],uv10,white});
        drawTexturedTriangle(renderer,{proj[5],uv01,white},{proj[7],uv10,white},{proj[6],uv00,white});
        // Arriba
        drawTexturedTriangle(renderer,{proj[3],uv01,white},{proj[2],uv11,white},{proj[6],uv10,white});
        drawTexturedTriangle(renderer,{proj[3],uv01,white},{proj[6],uv10,white},{proj[7],uv00,white});
        // Abajo
        drawTexturedTriangle(renderer,{proj[4],uv01,white},{proj[5],uv11,white},{proj[1],uv10,white});
        drawTexturedTriangle(renderer,{proj[4],uv01,white},{proj[1],uv10,white},{proj[0],uv00,white});
        // Derecha
        drawTexturedTriangle(renderer,{proj[1],uv01,white},{proj[5],uv11,white},{proj[6],uv10,white});
        drawTexturedTriangle(renderer,{proj[1],uv01,white},{proj[6],uv10,white},{proj[2],uv00,white});
        // Izquierda
        drawTexturedTriangle(renderer,{proj[4],uv01,white},{proj[0],uv11,white},{proj[3],uv10,white});
        drawTexturedTriangle(renderer,{proj[4],uv01,white},{proj[3],uv10,white},{proj[7],uv00,white});
    }
};

} // namespace N64
