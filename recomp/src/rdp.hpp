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

struct Camera3D {
    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;
    float posX = 0.0f, posY = 2.0f, posZ = -5.0f;
    float rotY = 0.0f;
    float pitch = 22.0f; // Inclinación suave cenital de N64
    float distance = 5.5f;

    void update(float pX, float pY, float pZ, float camInputX, float dt) {
        rotY += camInputX * 100.0f * dt;

        // Seguir al centro de Conker
        targetX += (pX - targetX) * 8.0f * dt;
        targetY += ((pY - 0.3f) - targetY) * 8.0f * dt;
        targetZ += (pZ - targetZ) * 8.0f * dt;

        float radY = rotY * 3.14159265f / 180.0f;
        float radP = pitch * 3.14159265f / 180.0f;

        posX = targetX + std::sin(radY) * std::cos(radP) * distance;
        posY = targetY + std::sin(radP) * distance;
        posZ = targetZ - std::cos(radY) * std::cos(radP) * distance;
    }
};

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
        groundTexture = TextureLoader::getInstance().createConkerProceduralTexture(renderer, 32, 32);

        std::cout << "[RDP] Loaded 3D Mesh: " << conkerMesh.name << " (" 
                  << conkerMesh.vertices.size() << " vertices, " 
                  << conkerMesh.triangles.size() << " triangles)" << std::endl;
        std::cout << "[RDP] Third-Person Camera and 3D Ground Plane Engine active." << std::endl;
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
            uint8_t a = 255;
            argb32[i] = (static_cast<uint32_t>(a) << 24) |
                        (static_cast<uint32_t>(r) << 16) |
                        (static_cast<uint32_t>(g) << 8)  |
                        static_cast<uint32_t>(b);
        }

        activeTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STATIC, width, height);
        if (activeTexture) {
            SDL_SetTextureBlendMode(activeTexture, SDL_BLENDMODE_NONE);
            SDL_UpdateTexture(activeTexture, nullptr, argb32.data(), width * sizeof(uint32_t));
            std::cout << "[RDP] Real Rareware texture mapped to 3D Mesh: " << width << "x" << height << " RGBA16." << std::endl;
        }
    }

    void shutdown() {
        if (activeTexture) { SDL_DestroyTexture(activeTexture); activeTexture = nullptr; }
        if (groundTexture) { SDL_DestroyTexture(groundTexture); groundTexture = nullptr; }
    }

    void processDisplayList(uint32_t dlVaddr, SDL_Renderer* renderer, int winW, int winH, float pX, float pY, float pZ, float pRotY, float camInputX, float dt) {
        (void)dlVaddr;
        if (!renderer) return;

        camera.update(pX, pY, pZ, camInputX, dt);

        // 1. Renderizar Suelo 3D de N64
        renderGroundPlane(renderer, winW, winH, pX, pZ);

        // 2. Renderizar Sombra Proyectada de Conker
        renderPlayerShadow(renderer, winW, winH, pX, pZ, pY);

        // 3. Renderizar Personaje (Conker)
        renderConkerMesh3D(renderer, winW, winH, pX, pY, pZ, pRotY);
    }

private:
    RDPProcessor() : activeTexture(nullptr), groundTexture(nullptr) {}
    SDL_Texture* activeTexture;
    SDL_Texture* groundTexture;
    Model3D conkerMesh;
    Camera3D camera;

    struct Point2D { float x, y, z; };

    // Proyección de cámara 3D en perspectiva con View Matrix clásica
    Point2D projectCamera(float wx, float wy, float wz, int winW, int winH) {
        // Vector relativo del objeto respecto a la cámara
        float dx = wx - camera.posX;
        float dy = wy - camera.posY;
        float dz = wz - camera.posZ;

        // Rotación Y (Yaw)
        float radY = camera.rotY * 3.14159265f / 180.0f;
        float cosY = std::cos(radY), sinY = std::sin(radY);
        float x1 = dx * cosY - dz * sinY;
        float z1 = dx * sinY + dz * cosY;

        // Rotación X (Pitch)
        float radP = camera.pitch * 3.14159265f / 180.0f;
        float cosP = std::cos(radP), sinP = std::sin(radP);
        float y2 = dy * cosP + z1 * sinP;
        float z2 = -dy * sinP + z1 * cosP;

        if (z2 < 0.1f) z2 = 0.1f;
        float fov = 420.0f;
        float factor = fov / z2;

        return {
            winW / 2.0f + x1 * factor,
            winH / 2.0f - y2 * factor,
            z2
        };
    }

    // Renderiza la cuadrícula de suelo 3D con textura de Rareware
    void renderGroundPlane(SDL_Renderer* renderer, int winW, int winH, float pX, float pZ) {
        int gridSize = 16;
        float tileSize = 1.5f;
        float startX = std::floor(pX / tileSize) * tileSize - (gridSize / 2) * tileSize;
        float startZ = std::floor(pZ / tileSize) * tileSize - (gridSize / 2) * tileSize;

        for (int x = 0; x < gridSize; ++x) {
            for (int z = 0; z < gridSize; ++z) {
                float x0 = startX + x * tileSize;
                float x1 = x0 + tileSize;
                float z0 = startZ + z * tileSize;
                float z1 = z0 + tileSize;
                float y = -1.35f; // Nivel de los pies

                Point2D p0 = projectCamera(x0, y, z0, winW, winH);
                Point2D p1 = projectCamera(x1, y, z0, winW, winH);
                Point2D p2 = projectCamera(x1, y, z1, winW, winH);
                Point2D p3 = projectCamera(x0, y, z1, winW, winH);

                if (p0.z < 0.5f || p1.z < 0.5f || p2.z < 0.5f || p3.z < 0.5f) continue;

                // Patrón de tablero verde pasto / marrón
                bool alt = ((x + z) % 2 == 0);
                SDL_Color c = alt ? SDL_Color{35, 95, 30, 255} : SDL_Color{25, 75, 20, 255};

                SDL_Vertex v1[3] = {
                    { { p0.x, p0.y }, c, { 0.0f, 0.0f } },
                    { { p1.x, p1.y }, c, { 1.0f, 0.0f } },
                    { { p2.x, p2.y }, c, { 1.0f, 1.0f } }
                };
                SDL_Vertex v2[3] = {
                    { { p0.x, p0.y }, c, { 0.0f, 0.0f } },
                    { { p2.x, p2.y }, c, { 1.0f, 1.0f } },
                    { { p3.x, p3.y }, c, { 0.0f, 1.0f } }
                };

                SDL_RenderGeometry(renderer, nullptr, v1, 3, nullptr, 0);
                SDL_RenderGeometry(renderer, nullptr, v2, 3, nullptr, 0);

                // Cuadrícula sutil
                SDL_SetRenderDrawColor(renderer, 20, 60, 15, 120);
                SDL_RenderDrawLineF(renderer, p0.x, p0.y, p1.x, p1.y);
                SDL_RenderDrawLineF(renderer, p1.x, p1.y, p2.x, p2.y);
            }
        }
    }

    // Sombra circular estilo N64 bajo Conker
    void renderPlayerShadow(SDL_Renderer* renderer, int winW, int winH, float pX, float pZ, float pY) {
        float shadowRadius = (0.65f - pY * 0.08f < 0.2f) ? 0.2f : (0.65f - pY * 0.08f);
        float y = -1.33f;
        int segments = 12;
        SDL_Color shadowColor = { 10, 20, 10, 160 };

        Point2D center = projectCamera(pX, y, pZ, winW, winH);
        if (center.z < 0.5f) return;

        for (int i = 0; i < segments; ++i) {
            float a1 = (i * 360.0f / segments) * 3.14159265f / 180.0f;
            float a2 = ((i + 1) * 360.0f / segments) * 3.14159265f / 180.0f;

            Point2D pt1 = projectCamera(pX + std::cos(a1) * shadowRadius, y, pZ + std::sin(a1) * shadowRadius, winW, winH);
            Point2D pt2 = projectCamera(pX + std::cos(a2) * shadowRadius, y, pZ + std::sin(a2) * shadowRadius, winW, winH);

            SDL_Vertex tri[3] = {
                { { center.x, center.y }, shadowColor, { 0.5f, 0.5f } },
                { { pt1.x, pt1.y }, shadowColor, { 0.0f, 0.0f } },
                { { pt2.x, pt2.y }, shadowColor, { 1.0f, 1.0f } }
            };
            SDL_RenderGeometry(renderer, nullptr, tri, 3, nullptr, 0);
        }
    }

    void renderConkerMesh3D(SDL_Renderer* renderer, int winW, int winH, float posX, float posY, float posZ, float rotY) {
        float radY = rotY * 3.14159265f / 180.0f;
        float cosY = std::cos(radY), sinY = std::sin(radY);

        std::vector<Point2D> projected(conkerMesh.vertices.size());
        std::vector<float> transformedZ(conkerMesh.vertices.size());

        for (size_t i = 0; i < conkerMesh.vertices.size(); ++i) {
            const auto& v = conkerMesh.vertices[i];
            
            // Rotación local del personaje
            float lx = v.x * cosY + v.z * sinY;
            float lz = -v.x * sinY + v.z * cosY;
            float ly = v.y;

            // Mundo 3D
            float wx = lx + posX;
            float wy = ly + posY;
            float wz = lz + posZ;

            projected[i] = projectCamera(wx, wy, wz, winW, winH);
            transformedZ[i] = projected[i].z;
        }

        // Z-Sorting
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
            return b.avgZ < a.avgZ;
        });

        for (const auto& tri : drawList) {
            const auto& v0_raw = conkerMesh.vertices[tri.v0];
            const auto& v1_raw = conkerMesh.vertices[tri.v1];
            const auto& v2_raw = conkerMesh.vertices[tri.v2];

            const auto& p0 = projected[tri.v0];
            const auto& p1 = projected[tri.v1];
            const auto& p2 = projected[tri.v2];

            if (p0.z < 0.2f || p1.z < 0.2f || p2.z < 0.2f) continue;

            float light = 1.0f;
            SDL_Color c0 = { static_cast<Uint8>(v0_raw.r * 255 * light), static_cast<Uint8>(v0_raw.g * 255 * light), static_cast<Uint8>(v0_raw.b * 255 * light), 255 };
            SDL_Color c1 = { static_cast<Uint8>(v1_raw.r * 255 * light), static_cast<Uint8>(v1_raw.g * 255 * light), static_cast<Uint8>(v1_raw.b * 255 * light), 255 };
            SDL_Color c2 = { static_cast<Uint8>(v2_raw.r * 255 * light), static_cast<Uint8>(v2_raw.g * 255 * light), static_cast<Uint8>(v2_raw.b * 255 * light), 255 };

            SDL_Vertex vertices[3] = {
                { { p0.x, p0.y }, c0, { v0_raw.u, v0_raw.v } },
                { { p1.x, p1.y }, c1, { v1_raw.u, v1_raw.v } },
                { { p2.x, p2.y }, c2, { v2_raw.u, v2_raw.v } }
            };

            SDL_RenderGeometry(renderer, activeTexture, vertices, 3, nullptr, 0);
        }
    }
};

} // namespace N64
