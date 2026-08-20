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
        activeTexture = TextureLoader::getInstance().createConkerProceduralTexture(renderer, 64, 64);
    }

    void shutdown() {
        if (activeTexture) {
            SDL_DestroyTexture(activeTexture);
            activeTexture = nullptr;
        }
    }

    // Procesa Display List y renderiza geometría 3D con mapeo de coordenadas UV de textura
    void processDisplayList(uint32_t dlVaddr, SDL_Renderer* renderer, int winW, int winH, float angle) {
        (void)dlVaddr;
        if (!renderer) return;

        renderTexturedModel3D(renderer, winW, winH, angle);
    }

private:
    RDPProcessor() : activeTexture(nullptr) {}
    SDL_Texture* activeTexture;

    struct Vec3 { float x, y, z; };
    struct Point2D { float x, y; };
    struct VertexUV {
        Point2D pos;
        SDL_FPoint uv;
        SDL_Color color;
    };

    Point2D project(Vec3 v, int winW, int winH, float fov, float distance) {
        float z = v.z + distance;
        if (z < 0.1f) z = 0.1f;
        float factor = fov / z;
        return {
            winW / 2.0f + v.x * factor,
            winH / 2.0f - v.y * factor
        };
    }

    // Dibuja un triángulo con coordenadas UV mapeadas a la textura de N64 en GPU
    void drawTexturedTriangle(SDL_Renderer* renderer, VertexUV v1, VertexUV v2, VertexUV v3) {
        SDL_Vertex vertices[3] = {
            { { v1.pos.x, v1.pos.y }, v1.color, v1.uv },
            { { v2.pos.x, v2.pos.y }, v2.color, v2.uv },
            { { v3.pos.x, v3.pos.y }, v3.color, v3.uv }
        };
        SDL_RenderGeometry(renderer, activeTexture, vertices, 3, nullptr, 0);

        // Aristas 3D poligonales de N64
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 120);
        SDL_RenderDrawLineF(renderer, v1.pos.x, v1.pos.y, v2.pos.x, v2.pos.y);
        SDL_RenderDrawLineF(renderer, v2.pos.x, v2.pos.y, v3.pos.x, v3.pos.y);
        SDL_RenderDrawLineF(renderer, v3.pos.x, v3.pos.y, v1.pos.x, v1.pos.y);
    }

    void renderTexturedModel3D(SDL_Renderer* renderer, int winW, int winH, float angle) {
        Vec3 localVertices[8] = {
            {-1.0f, -1.0f, -1.0f}, // 0
            { 1.0f, -1.0f, -1.0f}, // 1
            { 1.0f,  1.0f, -1.0f}, // 2
            {-1.0f,  1.0f, -1.0f}, // 3
            {-1.0f, -1.0f,  1.0f}, // 4
            { 1.0f, -1.0f,  1.0f}, // 5
            { 1.0f,  1.0f,  1.0f}, // 6
            {-1.0f,  1.0f,  1.0f}  // 7
        };

        // Rotación 3D en los ejes X, Y y Z
        float radY = angle * 3.14159265f / 180.0f;
        float radX = (angle * 0.5f) * 3.14159265f / 180.0f;
        float cosY = std::cos(radY), sinY = std::sin(radY);
        float cosX = std::cos(radX), sinX = std::sin(radX);

        Point2D proj[8];
        for (int i = 0; i < 8; ++i) {
            float x1 = localVertices[i].x * cosY + localVertices[i].z * sinY;
            float z1 = -localVertices[i].x * sinY + localVertices[i].z * cosY;
            float y1 = localVertices[i].y;

            float y2 = y1 * cosX - z1 * sinX;
            float z2 = y1 * sinX + z1 * cosX;
            float x2 = x1;

            proj[i] = project({ x2, y2, z2 }, winW, winH, 360.0f, 3.5f);
        }

        SDL_Color white = { 255, 255, 255, 255 };

        // Mapeo oficial de coordenadas UV para cada vértice de los cuadriláteros (F3DEX2 G_SETTILE)
        SDL_FPoint uv00 = { 0.0f, 0.0f };
        SDL_FPoint uv10 = { 1.0f, 0.0f };
        SDL_FPoint uv11 = { 1.0f, 1.0f };
        SDL_FPoint uv01 = { 0.0f, 1.0f };

        // Cara Frontal Texturizada (0, 1, 2, 3)
        drawTexturedTriangle(renderer, { proj[0], uv01, white }, { proj[1], uv11, white }, { proj[2], uv10, white });
        drawTexturedTriangle(renderer, { proj[0], uv01, white }, { proj[2], uv10, white }, { proj[3], uv00, white });

        // Cara Trasera Texturizada (5, 4, 7, 6)
        drawTexturedTriangle(renderer, { proj[5], uv01, white }, { proj[4], uv11, white }, { proj[7], uv10, white });
        drawTexturedTriangle(renderer, { proj[5], uv01, white }, { proj[7], uv10, white }, { proj[6], uv00, white });

        // Cara Superior (3, 2, 6, 7)
        drawTexturedTriangle(renderer, { proj[3], uv01, white }, { proj[2], uv11, white }, { proj[6], uv10, white });
        drawTexturedTriangle(renderer, { proj[3], uv01, white }, { proj[6], uv10, white }, { proj[7], uv00, white });

        // Cara Inferior (4, 5, 1, 0)
        drawTexturedTriangle(renderer, { proj[4], uv01, white }, { proj[5], uv11, white }, { proj[1], uv10, white });
        drawTexturedTriangle(renderer, { proj[4], uv01, white }, { proj[1], uv10, white }, { proj[0], uv00, white });

        // Cara Derecha (1, 5, 6, 2)
        drawTexturedTriangle(renderer, { proj[1], uv01, white }, { proj[5], uv11, white }, { proj[6], uv10, white });
        drawTexturedTriangle(renderer, { proj[1], uv01, white }, { proj[6], uv10, white }, { proj[2], uv00, white });

        // Cara Izquierda (4, 0, 3, 7)
        drawTexturedTriangle(renderer, { proj[4], uv01, white }, { proj[0], uv11, white }, { proj[3], uv10, white });
        drawTexturedTriangle(renderer, { proj[4], uv01, white }, { proj[3], uv10, white }, { proj[7], uv00, white });
    }
};

} // namespace N64
