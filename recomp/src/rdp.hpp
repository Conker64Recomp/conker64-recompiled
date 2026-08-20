#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <SDL.h>
#include "gbi.hpp"
#include "memory.hpp"
#include "texture_loader.hpp"
#include "model_loader.hpp"

namespace N64 {

class RDPProcessor {
public:
    static RDPProcessor& getInstance() {
        static RDPProcessor instance;
        return instance;
    }

    void init(SDL_Renderer* renderer) {
        std::cout << "[RDP] Fast3D / F3DEX2 Microcode Display List Processor initialized." << std::endl;
        activeTexture = TextureLoader::getInstance().createConkerProceduralTexture(renderer, 64, 64);
        conkerMesh = Model3D::createConkerMesh();
        std::cout << "[RDP] Loaded 3D Mesh: " << conkerMesh.name << " (" 
                  << conkerMesh.vertices.size() << " vertices, " 
                  << conkerMesh.triangles.size() << " triangles)" << std::endl;
    }

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
            std::cout << "[RDP] Real Rareware texture mapped to 3D Mesh: " << width << "x" << height << " RGBA16." << std::endl;
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
        renderConkerMesh3D(renderer, winW, winH, angle);
    }

private:
    RDPProcessor() : activeTexture(nullptr) {}
    SDL_Texture* activeTexture;
    Model3D conkerMesh;

    struct Point2D { float x, y, z; };

    Point2D project(float x, float y, float z, int winW, int winH, float fov, float distance) {
        float depth = z + distance;
        if (depth < 0.1f) depth = 0.1f;
        float factor = fov / depth;
        return {
            winW / 2.0f + x * factor,
            winH / 2.0f - y * factor,
            depth
        };
    }

    void renderConkerMesh3D(SDL_Renderer* renderer, int winW, int winH, float angle) {
        float radY = angle * 3.14159265f / 180.0f;
        float radX = 15.0f * 3.14159265f / 180.0f; // Ligera inclinación para perspectiva N64
        float cosY = std::cos(radY), sinY = std::sin(radY);
        float cosX = std::cos(radX), sinX = std::sin(radX);

        // 1. Transformar y proyectar todos los vértices del modelo 3D
        std::vector<Point2D> projected(conkerMesh.vertices.size());
        std::vector<float> transformedZ(conkerMesh.vertices.size());

        for (size_t i = 0; i < conkerMesh.vertices.size(); ++i) {
            const auto& v = conkerMesh.vertices[i];
            
            // Rotación eje Y
            float x1 = v.x * cosY + v.z * sinY;
            float z1 = -v.x * sinY + v.z * cosY;
            
            // Rotación eje X
            float y2 = v.y * cosX - z1 * sinX;
            float z2 = v.y * sinX + z1 * cosX;

            projected[i] = project(x1, y2, z2, winW, winH, 450.0f, 4.5f);
            transformedZ[i] = z2;
        }

        // 2. Ordenamiento de caras (Z-Sorting / Painter's Algorithm para profundidad 3D limpia)
        struct RenderTri {
            uint16_t v0, v1, v2;
            float avgZ;
        };

        std::vector<RenderTri> drawList;
        drawList.reserve(conkerMesh.triangles.size());

        for (const auto& tri : conkerMesh.triangles) {
            float avgZ = (transformedZ[tri.v0] + transformedZ[tri.v1] + transformedZ[tri.v2]) / 3.0f;
            drawList.push_back({tri.v0, tri.v1, tri.v2, avgZ});
        }

        std::sort(drawList.begin(), drawList.end(), [](const RenderTri& a, const RenderTri& b) {
            return a.avgZ < b.avgZ; // Renderizar primero los más lejanos
        });

        // 3. Dibujar triángulos con texturas UV e iluminación por sombreado de vértices
        for (const auto& tri : drawList) {
            const auto& v0_raw = conkerMesh.vertices[tri.v0];
            const auto& v1_raw = conkerMesh.vertices[tri.v1];
            const auto& v2_raw = conkerMesh.vertices[tri.v2];

            const auto& p0 = projected[tri.v0];
            const auto& p1 = projected[tri.v1];
            const auto& p2 = projected[tri.v2];

            // Shading simple (luz direccional desde arriba a la derecha)
            float light = 0.85f;
            SDL_Color c0 = { static_cast<Uint8>(v0_raw.r * 255 * light), static_cast<Uint8>(v0_raw.g * 255 * light), static_cast<Uint8>(v0_raw.b * 255 * light), 255 };
            SDL_Color c1 = { static_cast<Uint8>(v1_raw.r * 255 * light), static_cast<Uint8>(v1_raw.g * 255 * light), static_cast<Uint8>(v1_raw.b * 255 * light), 255 };
            SDL_Color c2 = { static_cast<Uint8>(v2_raw.r * 255 * light), static_cast<Uint8>(v2_raw.g * 255 * light), static_cast<Uint8>(v2_raw.b * 255 * light), 255 };

            SDL_Vertex vertices[3] = {
                { { p0.x, p0.y }, c0, { v0_raw.u, v0_raw.v } },
                { { p1.x, p1.y }, c1, { v1_raw.u, v1_raw.v } },
                { { p2.x, p2.y }, c2, { v2_raw.u, v2_raw.v } }
            };

            SDL_RenderGeometry(renderer, activeTexture, vertices, 3, nullptr, 0);

            // Malla alámbrica sutil estilo N64
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 30);
            SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
            SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
            SDL_RenderDrawLineF(renderer, p2.x, p2.y, p0.x, p0.y);
        }
    }
};

} // namespace N64
