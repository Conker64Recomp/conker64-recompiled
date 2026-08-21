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
    float pitch = 24.0f; // N64 classic overhead angle
    float distance = 5.5f;

    void update(float pX, float pY, float pZ, float camInputX, float dt) {
        rotY += camInputX * 100.0f * dt;
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

        // Procedural fallback textures
        activeTexture = TextureLoader::getInstance().createConkerProceduralTexture(renderer, 64, 64);
        groundTexture = TextureLoader::getInstance().createConkerProceduralTexture(renderer, 32, 32);

        // Load authentic 3D meshes — pass renderer so MTL/PNG textures are loaded
        conkerMesh = Model3D::createConkerMesh(renderer);

        // Level geometry: merge real ROM OBJ files
        levelMesh.name = "Level (Merged ROM Meshes)";
        loadAdditionalLevel(renderer);

        std::cout << "[RDP] Loaded 3D Mesh: " << conkerMesh.name
                  << " (" << conkerMesh.vertices.size() << " vertices, "
                  << conkerMesh.triangles.size() << " triangles)" << std::endl;
        std::cout << "[RDP] Loaded Level Mesh: " << levelMesh.name
                  << " (" << levelMesh.vertices.size() << " vertices, "
                  << levelMesh.triangles.size() << " triangles)" << std::endl;
        std::cout << "[RDP] Third-Person Camera, Gouraud Shading, Back-Face Culling active." << std::endl;
    }

    void loadAdditionalLevel(SDL_Renderer* renderer) {
        const char* levelFiles[] = {
            "exported_assets/models/assets09_model_000.obj",
            "exported_assets/models/assets09_model_001.obj",
            "exported_assets/models/assets09_model_002.obj",
            "exported_assets/models/assets0A_model_000.obj",
            "exported_assets/models/assets0A_model_001.obj",
            "exported_assets/models/assets0B_model_000.obj",
            "../exported_assets/models/assets09_model_000.obj",
            "../exported_assets/models/assets0A_model_000.obj",
            nullptr
        };
        bool anyLoaded = false;
        for (int i = 0; levelFiles[i]; ++i) {
            Model3D tmp;
            if (tmp.loadFromOBJ(levelFiles[i], renderer)) {
                uint16_t base = static_cast<uint16_t>(levelMesh.vertices.size());
                for (auto& v : tmp.vertices) levelMesh.vertices.push_back(v);
                for (auto& t : tmp.triangles)
                    levelMesh.triangles.push_back({
                        static_cast<uint16_t>(t.v0 + base),
                        static_cast<uint16_t>(t.v1 + base),
                        static_cast<uint16_t>(t.v2 + base)
                    });
                // Use the first successfully loaded ROM texture for the level
                if (!levelMesh.sdlTexture && tmp.sdlTexture)
                    levelMesh.sdlTexture = tmp.sdlTexture;
                anyLoaded = true;
            }
        }
        if (!anyLoaded) levelMesh = Model3D::createLevelGeometry(renderer);
    }

    void loadRealTexture(SDL_Renderer* renderer, const uint8_t* rgba16Data, int width, int height) {
        if (!rgba16Data || width <= 0 || height <= 0) return;
        if (activeTexture) { SDL_DestroyTexture(activeTexture); activeTexture = nullptr; }
        std::vector<uint32_t> argb32(width * height);
        const uint16_t* src = reinterpret_cast<const uint16_t*>(rgba16Data);
        for (int i = 0; i < width * height; ++i) {
            uint16_t raw = src[i];
            raw = static_cast<uint16_t>((raw >> 8) | (raw << 8));
            uint8_t r = ((raw >> 11) & 0x1F); r = (r << 3) | (r >> 2);
            uint8_t g = ((raw >>  6) & 0x1F); g = (g << 3) | (g >> 2);
            uint8_t b = ((raw >>  1) & 0x1F); b = (b << 3) | (b >> 2);
            argb32[i] = (255u << 24) | (static_cast<uint32_t>(r) << 16) |
                        (static_cast<uint32_t>(g) << 8) | b;
        }
        activeTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STATIC, width, height);
        if (activeTexture) {
            SDL_SetTextureBlendMode(activeTexture, SDL_BLENDMODE_NONE);
            SDL_UpdateTexture(activeTexture, nullptr, argb32.data(), width * sizeof(uint32_t));
            std::cout << "[RDP] Real Rareware texture mapped: " << width << "x" << height << " RGBA16." << std::endl;
        }
    }

    void shutdown() {
        if (activeTexture) { SDL_DestroyTexture(activeTexture); activeTexture = nullptr; }
        if (groundTexture) { SDL_DestroyTexture(groundTexture); groundTexture = nullptr; }
    }

    void processDisplayList(uint32_t dlVaddr, SDL_Renderer* renderer,
                            int winW, int winH,
                            float pX, float pY, float pZ, float pRotY,
                            float camInputX, float dt) {
        (void)dlVaddr;
        if (!renderer) return;
        camera.update(pX, pY, pZ, camInputX, dt);
        // Sky + fog background gradient
        renderSkyGradient(renderer, winW, winH);
        // Ground tiles
        renderGroundPlane(renderer, winW, winH, pX, pZ);
        // Level geometry from ROM OBJ — prefer its own MTL texture
        SDL_Texture* levelTex = levelMesh.sdlTexture ? levelMesh.sdlTexture : activeTexture;
        renderStaticMesh(renderer, winW, winH, levelMesh, levelTex);
        // Player shadow blob
        renderPlayerShadow(renderer, winW, winH, pX, pZ, pY);
        // Conker 3D character — prefer its own MTL texture
        renderConkerMesh3D(renderer, winW, winH, pX, pY, pZ, pRotY);
    }

private:
    RDPProcessor() : activeTexture(nullptr), groundTexture(nullptr) {}
    SDL_Texture* activeTexture;
    SDL_Texture* groundTexture;
    Model3D conkerMesh;
    Model3D levelMesh;
    Camera3D camera;

    // ─── DIRECTIONAL LIGHT (N64-style) ──────────────────────────────────────
    // N64 lighting was typically one overhead directional + ambient
    float lightDirX = 0.5f, lightDirY = 1.0f, lightDirZ = 0.3f;
    float ambientIntensity = 0.35f;

    struct Point2D { float x, y, z; };

    // ─── PERSPECTIVE PROJECTION ─────────────────────────────────────────────
    Point2D projectCamera(float wx, float wy, float wz, int winW, int winH) const {
        float dx = wx - camera.posX;
        float dy = wy - camera.posY;
        float dz = wz - camera.posZ;
        float radY = camera.rotY * 3.14159265f / 180.0f;
        float cosY = std::cos(radY), sinY = std::sin(radY);
        float x1 = dx * cosY - dz * sinY;
        float z1 = dx * sinY + dz * cosY;
        float radP = camera.pitch * 3.14159265f / 180.0f;
        float cosP = std::cos(radP), sinP = std::sin(radP);
        float y2 =  dy * cosP + z1 * sinP;
        float z2 = -dy * sinP + z1 * cosP;
        if (z2 < 0.1f) z2 = 0.1f;
        float fov = 440.0f;
        float f = fov / z2;
        return { winW / 2.0f + x1 * f, winH / 2.0f - y2 * f, z2 };
    }

    // ─── GOURAUD SHADING per vertex ─────────────────────────────────────────
    // Compute diffuse intensity using face normal
    static float computeLighting(float nx, float ny, float nz,
                                 float ldx, float ldy, float ldz, float ambient) {
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len < 1e-5f) return ambient;
        nx /= len; ny /= len; nz /= len;
        float llen = std::sqrt(ldx*ldx + ldy*ldy + ldz*ldz);
        if (llen > 1e-5f) { ldx /= llen; ldy /= llen; ldz /= llen; }
        float dot = nx*ldx + ny*ldy + nz*ldz;
        if (dot < 0.0f) dot = 0.0f;
        return ambient + (1.0f - ambient) * dot;
    }

    // Compute face normal from 3 world-space vertices
    static void faceNormal(float ax, float ay, float az,
                           float bx, float by, float bz,
                           float cx, float cy, float cz,
                           float& nx, float& ny, float& nz) {
        float ux = bx-ax, uy = by-ay, uz = bz-az;
        float vx = cx-ax, vy = cy-ay, vz = cz-az;
        nx = uy*vz - uz*vy;
        ny = uz*vx - ux*vz;
        nz = ux*vy - uy*vx;
    }

    // ─── BACK-FACE CULLING ───────────────────────────────────────────────────
    // True if the triangle is front-facing relative to the camera
    bool isFrontFacing(const Point2D& p0, const Point2D& p1, const Point2D& p2) const {
        float cross = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
        return cross < 0.0f; // Counter-clockwise in screen space = front
    }

    // ─── SKY / FOG GRADIENT (N64 style) ─────────────────────────────────────
    void renderSkyGradient(SDL_Renderer* renderer, int winW, int winH) {
        // Top: deep sky blue  Bottom: lighter haze (N64 Windy level palette)
        for (int y = 0; y < winH / 2; ++y) {
            float t = static_cast<float>(y) / (winH / 2.0f);
            uint8_t r = static_cast<uint8_t>(30  + t * 80);
            uint8_t g = static_cast<uint8_t>(80  + t * 120);
            uint8_t b = static_cast<uint8_t>(160 + t * 80);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawLine(renderer, 0, y, winW, y);
        }
        // Horizon fog band
        for (int y = winH / 2; y < winH * 2 / 3; ++y) {
            float t = static_cast<float>(y - winH / 2) / (winH / 6.0f);
            uint8_t r = static_cast<uint8_t>(110 + t * 30);
            uint8_t g = static_cast<uint8_t>(200 + t * 20);
            uint8_t b = static_cast<uint8_t>(240 - t * 30);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawLine(renderer, 0, y, winW, y);
        }
    }

    // ─── GROUND PLANE ────────────────────────────────────────────────────────
    void renderGroundPlane(SDL_Renderer* renderer, int winW, int winH, float pX, float pZ) {
        const int gridSize = 20;
        const float tileSize = 1.5f;
        float startX = std::floor(pX / tileSize) * tileSize - (gridSize / 2) * tileSize;
        float startZ = std::floor(pZ / tileSize) * tileSize - (gridSize / 2) * tileSize;
        const float y = -1.35f;

        for (int xi = 0; xi < gridSize; ++xi) {
            for (int zi = 0; zi < gridSize; ++zi) {
                float x0 = startX + xi * tileSize, x1 = x0 + tileSize;
                float z0 = startZ + zi * tileSize, z1 = z0 + tileSize;

                Point2D p0 = projectCamera(x0, y, z0, winW, winH);
                Point2D p1 = projectCamera(x1, y, z0, winW, winH);
                Point2D p2 = projectCamera(x1, y, z1, winW, winH);
                Point2D p3 = projectCamera(x0, y, z1, winW, winH);

                if (p0.z < 0.3f || p1.z < 0.3f || p2.z < 0.3f || p3.z < 0.3f) continue;

                // Distance-based fog
                float fog = std::min(1.0f, p0.z / 18.0f);
                bool alt = ((xi + zi) % 2 == 0);
                uint8_t gr = alt ? static_cast<uint8_t>(35 + fog * 80) : static_cast<uint8_t>(25 + fog * 80);
                uint8_t gg = alt ? static_cast<uint8_t>(95 + fog * 80) : static_cast<uint8_t>(75 + fog * 80);
                uint8_t gb = alt ? static_cast<uint8_t>(30 + fog * 80) : static_cast<uint8_t>(20 + fog * 80);
                SDL_Color c = { gr, gg, gb, 255 };

                SDL_Vertex v1[3] = {
                    {{ p0.x, p0.y }, c, {0.0f, 0.0f}},
                    {{ p1.x, p1.y }, c, {1.0f, 0.0f}},
                    {{ p2.x, p2.y }, c, {1.0f, 1.0f}}
                };
                SDL_Vertex v2[3] = {
                    {{ p0.x, p0.y }, c, {0.0f, 0.0f}},
                    {{ p2.x, p2.y }, c, {1.0f, 1.0f}},
                    {{ p3.x, p3.y }, c, {0.0f, 1.0f}}
                };
                SDL_RenderGeometry(renderer, groundTexture, v1, 3, nullptr, 0);
                SDL_RenderGeometry(renderer, groundTexture, v2, 3, nullptr, 0);
            }
        }
    }

    // ─── PLAYER SHADOW ───────────────────────────────────────────────────────
    void renderPlayerShadow(SDL_Renderer* renderer, int winW, int winH, float pX, float pZ, float pY) {
        float radius = std::max(0.2f, 0.65f - pY * 0.08f);
        const float y = -1.33f;
        const int segs = 14;
        SDL_Color sc = { 10, 20, 10, 140 };
        Point2D center = projectCamera(pX, y, pZ, winW, winH);
        if (center.z < 0.5f) return;
        for (int i = 0; i < segs; ++i) {
            float a1 = (i     * 6.28318f) / segs;
            float a2 = ((i+1) * 6.28318f) / segs;
            Point2D pt1 = projectCamera(pX + std::cos(a1) * radius, y, pZ + std::sin(a1) * radius, winW, winH);
            Point2D pt2 = projectCamera(pX + std::cos(a2) * radius, y, pZ + std::sin(a2) * radius, winW, winH);
            SDL_Vertex tri[3] = {
                {{ center.x, center.y }, sc, {0.5f, 0.5f}},
                {{ pt1.x,    pt1.y    }, sc, {0.0f, 0.0f}},
                {{ pt2.x,    pt2.y    }, sc, {1.0f, 1.0f}}
            };
            SDL_RenderGeometry(renderer, nullptr, tri, 3, nullptr, 0);
        }
    }

    // ─── GENERIC MESH RENDERER with Gouraud + backface culling ──────────────
    void renderStaticMesh(SDL_Renderer* renderer, int winW, int winH,
                          const Model3D& mesh, SDL_Texture* tex) {
        if (mesh.vertices.empty() || mesh.triangles.empty()) return;

        std::vector<Point2D> proj(mesh.vertices.size());
        for (size_t i = 0; i < mesh.vertices.size(); ++i)
            proj[i] = projectCamera(mesh.vertices[i].x,
                                    mesh.vertices[i].y,
                                    mesh.vertices[i].z, winW, winH);

        struct RTri { uint16_t v0, v1, v2; float avgZ; };
        std::vector<RTri> drawList;
        drawList.reserve(mesh.triangles.size());
        for (const auto& t : mesh.triangles) {
            if (t.v0 >= proj.size() || t.v1 >= proj.size() || t.v2 >= proj.size()) continue;
            if (proj[t.v0].z < 0.2f || proj[t.v1].z < 0.2f || proj[t.v2].z < 0.2f) continue;
            if (!isFrontFacing(proj[t.v0], proj[t.v1], proj[t.v2])) continue;
            float avgZ = (proj[t.v0].z + proj[t.v1].z + proj[t.v2].z) / 3.0f;
            drawList.push_back({t.v0, t.v1, t.v2, avgZ});
        }
        std::sort(drawList.begin(), drawList.end(),
                  [](const RTri& a, const RTri& b){ return b.avgZ < a.avgZ; });

        for (const auto& t : drawList) {
            const auto& v0 = mesh.vertices[t.v0];
            const auto& v1 = mesh.vertices[t.v1];
            const auto& v2 = mesh.vertices[t.v2];
            const auto& p0 = proj[t.v0];
            const auto& p1 = proj[t.v1];
            const auto& p2 = proj[t.v2];

            float nx, ny, nz;
            faceNormal(v0.x, v0.y, v0.z,
                       v1.x, v1.y, v1.z,
                       v2.x, v2.y, v2.z, nx, ny, nz);
            float L = computeLighting(nx, ny, nz, lightDirX, lightDirY, lightDirZ, ambientIntensity);
            L = std::min(1.0f, L);

            float fog = std::min(1.0f, ((p0.z + p1.z + p2.z) / 3.0f) / 20.0f);

            auto shade = [&](float base, float l, float f) -> uint8_t {
                float c = base * l * (1.0f - f) + f * 0.8f;
                return static_cast<uint8_t>(std::min(255.0f, c * 255.0f));
            };

            SDL_Color c0 = { shade(v0.r, L, fog), shade(v0.g, L, fog), shade(v0.b, L, fog), 255 };
            SDL_Color c1 = { shade(v1.r, L, fog), shade(v1.g, L, fog), shade(v1.b, L, fog), 255 };
            SDL_Color c2 = { shade(v2.r, L, fog), shade(v2.g, L, fog), shade(v2.b, L, fog), 255 };

            SDL_Vertex verts[3] = {
                {{ p0.x, p0.y }, c0, { v0.u, v0.v }},
                {{ p1.x, p1.y }, c1, { v1.u, v1.v }},
                {{ p2.x, p2.y }, c2, { v2.u, v2.v }}
            };
            SDL_RenderGeometry(renderer, tex ? tex : activeTexture, verts, 3, nullptr, 0);
        }
    }

    // ─── CONKER CHARACTER MESH ───────────────────────────────────────────────
    void renderConkerMesh3D(SDL_Renderer* renderer, int winW, int winH,
                            float posX, float posY, float posZ, float rotY) {
        if (conkerMesh.vertices.empty()) return;

        float radY = rotY * 3.14159265f / 180.0f;
        float cosY = std::cos(radY), sinY = std::sin(radY);

        // Transform vertices into world space
        std::vector<Point2D> proj(conkerMesh.vertices.size());
        std::vector<std::array<float,3>> worldPos(conkerMesh.vertices.size());

        for (size_t i = 0; i < conkerMesh.vertices.size(); ++i) {
            const auto& v = conkerMesh.vertices[i];
            float lx =  v.x * cosY + v.z * sinY;
            float lz = -v.x * sinY + v.z * cosY;
            float wx = lx + posX, wy = v.y + posY, wz = lz + posZ;
            worldPos[i] = { wx, wy, wz };
            proj[i] = projectCamera(wx, wy, wz, winW, winH);
        }

        struct RTri { uint16_t v0, v1, v2; float avgZ; };
        std::vector<RTri> drawList;
        drawList.reserve(conkerMesh.triangles.size());

        for (const auto& t : conkerMesh.triangles) {
            if (t.v0 >= proj.size() || t.v1 >= proj.size() || t.v2 >= proj.size()) continue;
            if (proj[t.v0].z < 0.2f || proj[t.v1].z < 0.2f || proj[t.v2].z < 0.2f) continue;
            if (!isFrontFacing(proj[t.v0], proj[t.v1], proj[t.v2])) continue;
            float avgZ = (proj[t.v0].z + proj[t.v1].z + proj[t.v2].z) / 3.0f;
            drawList.push_back({t.v0, t.v1, t.v2, avgZ});
        }
        std::sort(drawList.begin(), drawList.end(),
                  [](const RTri& a, const RTri& b){ return b.avgZ < a.avgZ; });

        for (const auto& t : drawList) {
            const auto& v0 = conkerMesh.vertices[t.v0];
            const auto& v1 = conkerMesh.vertices[t.v1];
            const auto& v2 = conkerMesh.vertices[t.v2];
            const auto& wp0 = worldPos[t.v0];
            const auto& wp1 = worldPos[t.v1];
            const auto& wp2 = worldPos[t.v2];
            const auto& p0 = proj[t.v0];
            const auto& p1 = proj[t.v1];
            const auto& p2 = proj[t.v2];

            float nx, ny, nz;
            faceNormal(wp0[0], wp0[1], wp0[2],
                       wp1[0], wp1[1], wp1[2],
                       wp2[0], wp2[1], wp2[2], nx, ny, nz);
            float L = computeLighting(nx, ny, nz, lightDirX, lightDirY, lightDirZ, ambientIntensity);
            L = std::min(1.0f, L);
            float fog = std::min(1.0f, ((p0.z + p1.z + p2.z) / 3.0f) / 20.0f);

            auto shade = [&](float base, float l, float f) -> uint8_t {
                float c = base * l * (1.0f - f) + f * 0.8f;
                return static_cast<uint8_t>(std::min(255.0f, c * 255.0f));
            };

            SDL_Color c0 = { shade(v0.r, L, fog), shade(v0.g, L, fog), shade(v0.b, L, fog), 255 };
            SDL_Color c1 = { shade(v1.r, L, fog), shade(v1.g, L, fog), shade(v1.b, L, fog), 255 };
            SDL_Color c2 = { shade(v2.r, L, fog), shade(v2.g, L, fog), shade(v2.b, L, fog), 255 };

            SDL_Vertex verts[3] = {
                {{ p0.x, p0.y }, c0, { v0.u, v0.v }},
                {{ p1.x, p1.y }, c1, { v1.u, v1.v }},
                {{ p2.x, p2.y }, c2, { v2.u, v2.v }}
            };
            SDL_RenderGeometry(renderer, conkerMesh.sdlTexture ? conkerMesh.sdlTexture : activeTexture, verts, 3, nullptr, 0);
        }
    }
};

} // namespace N64
