#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <limits>
#include <SDL.h>
#include "gbi.hpp"
#include "memory.hpp"
#include "texture_loader.hpp"
#include "model_loader.hpp"
#include "actor_system.hpp"
#include "asset_paths.hpp"

namespace N64 {

template<typename T>
inline T mathMin(T a, T b) { return (a < b) ? a : b; }

template<typename T>
inline T mathMax(T a, T b) { return (a > b) ? a : b; }

template<typename T>
inline T mathClamp(T val, T low, T high) {
    return (val < low) ? low : ((val > high) ? high : val);
}

constexpr float kPi = 3.14159265f;
constexpr float kDegToRad = kPi / 180.0f;

struct Camera3D {
    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;
    float posX = 0.0f, posY = 2.0f, posZ = -5.0f;
    float rotY = 0.0f;
    float pitch = 24.0f; // N64 classic overhead angle
    float distance = 5.5f;

    // El giro se integra por separado del seguimiento para que el sistema de
    // actores pueda leer el yaw ya actualizado y orientar el movimiento del
    // jugador respecto a la camara.
    void advanceYaw(float camInputX, float dt) {
        rotY += camInputX * 100.0f * dt;
    }

    void follow(float pX, float pY, float pZ, float dt) {
        targetX += (pX - targetX) * 8.0f * dt;
        targetY += ((pY + 0.2f) - targetY) * 8.0f * dt;
        targetZ += (pZ - targetZ) * 8.0f * dt;
        float radY = rotY * kDegToRad;
        float radP = pitch * kDegToRad;
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
        std::cout << "[RDP] Fast3D / F3DEX2 Display List Processor initialized." << std::endl;

        activeTexture = TextureLoader::getInstance().createConkerProceduralTexture(renderer, 64, 64);
        groundTexture = TextureLoader::getInstance().createWindyGrassTexture(renderer, 256, 256);

        conkerMesh = Model3D::createConkerMesh(renderer);
        loadLevelGeometry(renderer);

        std::cout << "[RDP] Player mesh: " << conkerMesh.name
                  << " (" << conkerMesh.vertices.size() << " verts, "
                  << conkerMesh.triangles.size() << " tris)" << std::endl;
        std::cout << "[RDP] Level mesh: " << levelMesh.name
                  << " (" << levelMesh.vertices.size() << " verts, "
                  << levelMesh.triangles.size() << " tris)" << std::endl;
    }

    // Los modelos de la ROM se resuelven contra el directorio de assets, no
    // contra el cwd; antes ninguna de las rutas relativas acertaba desde
    // recomp/build y siempre caia al fallback procedural.
    void loadLevelGeometry(SDL_Renderer* renderer) {
        static const char* kLevelModels[] = {
            "models/assets09_model_483.obj",
            "models/assets10_model_069.obj",
            "models/assets0E_model_069.obj",
            "models/assets02_model_000.obj",
            "models/assets04_model_001.obj",
            "models/assets0A_model_000.obj",
            nullptr
        };

        Model3D merged;
        merged.name = "Level (ROM Meshes)";
        int loadedCount = 0;

        for (int i = 0; kLevelModels[i]; ++i) {
            std::string path = AssetPaths::getInstance().resolve(kLevelModels[i]);
            if (path.empty()) continue;

            Model3D piece;
            if (!piece.loadFromOBJ(path, renderer)) continue;

            uint32_t base = static_cast<uint32_t>(merged.vertices.size());
            merged.vertices.insert(merged.vertices.end(), piece.vertices.begin(), piece.vertices.end());
            for (const auto& t : piece.triangles) {
                merged.triangles.push_back({ t.v0 + base, t.v1 + base, t.v2 + base });
            }

            // La primera textura valida representa al conjunto; las demas se
            // liberan para no filtrar objetos SDL.
            if (!merged.sdlTexture && piece.sdlTexture) {
                merged.sdlTexture = piece.sdlTexture;
                piece.sdlTexture = nullptr;
            }
            piece.releaseTexture();
            ++loadedCount;
        }

        const size_t rawTris = merged.triangles.size();
        pruneDegenerateTriangles(merged);
        const float flatness = verticalFlatness(merged);

        std::cout << "[RDP] ROM level geometry: " << loadedCount << " models, "
                  << rawTris << " tris -> " << merged.triangles.size()
                  << " non-degenerate, vertical extent " << (flatness * 100.0f) << "% of footprint."
                  << std::endl;

        // Un nivel tiene relieve. Si la malla es practicamente una lamina plana
        // es ruido del extractor, no geometria: dibujarla solo produce las
        // franjas que se ven tumbadas en el suelo.
        if (merged.triangles.size() >= kMinLevelTriangles && flatness >= kMinFlatness) {
            fitMeshToWorld(merged, 18.0f);
            levelMesh = std::move(merged);
            std::cout << "[RDP] Using ROM level geometry ("
                      << levelMesh.triangles.size() << " tris)." << std::endl;
            return;
        }

        std::cout << "[RDP] ROM level geometry rejected; using procedural level "
                  << "until the F3DEX2 extractor lands." << std::endl;
        merged.releaseTexture();
        levelMesh = Model3D::createLevelGeometry(renderer);
    }

    void loadRealTexture(SDL_Renderer* renderer, const uint8_t* rgba16Data, int width, int height) {
        if (!rgba16Data || width <= 0 || height <= 0) return;
        if (activeTexture) { SDL_DestroyTexture(activeTexture); activeTexture = nullptr; }
        std::vector<uint32_t> argb32(static_cast<size_t>(width) * height);
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
            std::cout << "[RDP] Rareware RGBA16 texture mapped: " << width << "x" << height << std::endl;
        }
    }

    void shutdown() {
        if (activeTexture) { SDL_DestroyTexture(activeTexture); activeTexture = nullptr; }
        if (groundTexture) { SDL_DestroyTexture(groundTexture); groundTexture = nullptr; }
        conkerMesh.releaseTexture();
        levelMesh.releaseTexture();
    }

    // Integra el giro de camara antes de mover al jugador, para que el actor
    // pueda orientar su input respecto al yaw ya vigente en este frame.
    void advanceCameraYaw(float camInputX, float dt) {
        camera.advanceYaw(camInputX, dt);
    }

    float getCameraYaw() const { return camera.rotY; }

    void processDisplayList(uint32_t dlVaddr, SDL_Renderer* renderer,
                            int winW, int winH,
                            const ActorState& player, float dt) {
        (void)dlVaddr;
        if (!renderer) return;

        camera.follow(player.posX, player.posY, player.posZ, dt);
        cacheCameraBasis();

        renderSkyGradient(renderer, winW, winH);
        renderGroundPlane(renderer, winW, winH, player.posX, player.posZ);

        // Solo la textura propia de la malla. Antes se caia a `activeTexture`
        // (la RGBA16 suelta de la ROM), que sobre UVs de cajas procedurales se
        // veia como ruido estatico encima del personaje y del nivel.
        renderStaticMesh(renderer, winW, winH, levelMesh, levelMesh.sdlTexture);

        renderPlayerShadow(renderer, winW, winH, player.posX, player.posZ, player.posY);
        renderConkerMesh3D(renderer, winW, winH, player);
    }

private:
    RDPProcessor() : activeTexture(nullptr), groundTexture(nullptr) {}

    static constexpr size_t kMinLevelTriangles = 64;
    static constexpr float  kMinFlatness       = 0.05f;  // altura minima vs. huella

    SDL_Texture* activeTexture;
    SDL_Texture* groundTexture;
    Model3D conkerMesh;
    Model3D levelMesh;
    Camera3D camera;

    float lightDirX = 0.5f, lightDirY = 1.0f, lightDirZ = 0.3f;
    float ambientIntensity = 0.35f;

    // Base de camara cacheada una vez por frame. Antes projectCamera calculaba
    // sin/cos por cada vertice proyectado.
    float camCosY = 1.0f, camSinY = 0.0f;
    float camCosP = 1.0f, camSinP = 0.0f;

    struct Point2D { float x, y, z; };
    struct RTri { uint32_t v0, v1, v2; float avgZ; };

    // Buffers reutilizados entre frames para evitar rotacion de heap.
    std::vector<Point2D>             projScratch;
    std::vector<std::array<float,3>> worldScratch;
    std::vector<RTri>                drawScratch;
    std::vector<SDL_Vertex>          batchScratch;

    void cacheCameraBasis() {
        float radY = camera.rotY * kDegToRad;
        float radP = camera.pitch * kDegToRad;
        camCosY = std::cos(radY);
        camSinY = std::sin(radY);
        camCosP = std::cos(radP);
        camSinP = std::sin(radP);
    }

    // Los OBJ del extractor traen multitud de triangulos con indices repetidos
    // o vertices colineales. Dibujarlos no aporta superficie y satura el
    // z-sort, asi que se descartan al cargar.
    static void pruneDegenerateTriangles(Model3D& mesh) {
        std::vector<Triangle3D> kept;
        kept.reserve(mesh.triangles.size());
        const size_t n = mesh.vertices.size();

        for (const auto& t : mesh.triangles) {
            if (t.v0 == t.v1 || t.v1 == t.v2 || t.v0 == t.v2) continue;
            if (t.v0 >= n || t.v1 >= n || t.v2 >= n) continue;

            const auto& a = mesh.vertices[t.v0];
            const auto& b = mesh.vertices[t.v1];
            const auto& c = mesh.vertices[t.v2];

            float nx, ny, nz;
            faceNormal(a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, nx, ny, nz);
            if (nx * nx + ny * ny + nz * nz < 1e-8f) continue;   // area nula

            kept.push_back(t);
        }
        mesh.triangles.swap(kept);
    }

    // Altura de la malla relativa a su huella horizontal. ~0 = lamina plana.
    static float verticalFlatness(const Model3D& mesh) {
        if (mesh.vertices.empty()) return 0.0f;

        float minX = std::numeric_limits<float>::max(), maxX = -minX;
        float minY = minX, maxY = -minX;
        float minZ = minX, maxZ = -minX;
        for (const auto& v : mesh.vertices) {
            minX = mathMin(minX, v.x); maxX = mathMax(maxX, v.x);
            minY = mathMin(minY, v.y); maxY = mathMax(maxY, v.y);
            minZ = mathMin(minZ, v.z); maxZ = mathMax(maxZ, v.z);
        }

        float footprint = mathMax(maxX - minX, maxZ - minZ);
        if (footprint < 1e-4f) return 0.0f;
        return (maxY - minY) / footprint;
    }

    // Centra la malla en el origen y la escala a un radio horizontal util.
    static void fitMeshToWorld(Model3D& mesh, float targetRadius) {
        if (mesh.vertices.empty()) return;

        float minX = std::numeric_limits<float>::max(), maxX = -minX;
        float minY = minX, maxY = -minX;
        float minZ = minX, maxZ = -minX;
        for (const auto& v : mesh.vertices) {
            minX = mathMin(minX, v.x); maxX = mathMax(maxX, v.x);
            minY = mathMin(minY, v.y); maxY = mathMax(maxY, v.y);
            minZ = mathMin(minZ, v.z); maxZ = mathMax(maxZ, v.z);
        }
        (void)maxY;

        float extent = mathMax(maxX - minX, maxZ - minZ);
        if (extent < 1e-4f) return;

        float scale = (targetRadius * 2.0f) / extent;
        float cx = (minX + maxX) * 0.5f;
        float cz = (minZ + maxZ) * 0.5f;

        for (auto& v : mesh.vertices) {
            v.x = (v.x - cx) * scale;
            v.y = (v.y - minY) * scale;   // apoya la malla sobre el plano y=0
            v.z = (v.z - cz) * scale;
        }
    }

    Point2D projectCamera(float wx, float wy, float wz, int winW, int winH) const {
        float dx = wx - camera.posX;
        float dy = wy - camera.posY;
        float dz = wz - camera.posZ;
        float x1 = dx * camCosY - dz * camSinY;
        float z1 = dx * camSinY + dz * camCosY;
        float y2 =  dy * camCosP + z1 * camSinP;
        float z2 = -dy * camSinP + z1 * camCosP;
        if (z2 < 0.1f) z2 = 0.1f;
        constexpr float fov = 440.0f;
        float f = fov / z2;
        return { winW / 2.0f + x1 * f, winH / 2.0f - y2 * f, z2 };
    }

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

    static bool isFrontFacing(const Point2D& p0, const Point2D& p1, const Point2D& p2) {
        float cross = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
        return cross < 0.0f;
    }

    static uint8_t shadeVal(float base, float l, float f) {
        float c = base * l * (1.0f - f) + f * 0.8f;
        return static_cast<uint8_t>(mathMin(255.0f, c * 255.0f));
    }

    void renderSkyGradient(SDL_Renderer* renderer, int winW, int winH) {
        for (int y = 0; y < winH / 2; ++y) {
            float t = static_cast<float>(y) / (winH / 2.0f);
            uint8_t r = static_cast<uint8_t>(30  + t * 80);
            uint8_t g = static_cast<uint8_t>(80  + t * 120);
            uint8_t b = static_cast<uint8_t>(160 + t * 80);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawLine(renderer, 0, y, winW, y);
        }
        for (int y = winH / 2; y < winH * 2 / 3; ++y) {
            float t = static_cast<float>(y - winH / 2) / (winH / 6.0f);
            uint8_t r = static_cast<uint8_t>(110 + t * 30);
            uint8_t g = static_cast<uint8_t>(200 + t * 20);
            uint8_t b = static_cast<uint8_t>(240 - t * 30);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawLine(renderer, 0, y, winW, y);
        }
    }

    // 400 baldosas se emiten en UNA llamada de dibujo; antes eran 800
    // SDL_RenderGeometry por frame, una por triangulo.
    void renderGroundPlane(SDL_Renderer* renderer, int winW, int winH, float pX, float pZ) {
        const int gridSize = 20;
        const float tileSize = 1.5f;
        float startX = std::floor(pX / tileSize) * tileSize - (gridSize / 2) * tileSize;
        float startZ = std::floor(pZ / tileSize) * tileSize - (gridSize / 2) * tileSize;
        const float y = 0.0f;

        batchScratch.clear();
        batchScratch.reserve(static_cast<size_t>(gridSize) * gridSize * 6);

        constexpr float kUvScale = 1.0f / 4.5f; // Mapeo de textura continuo en espacio de mundo

        for (int xi = 0; xi < gridSize; ++xi) {
            for (int zi = 0; zi < gridSize; ++zi) {
                float x0 = startX + xi * tileSize, x1 = x0 + tileSize;
                float z0 = startZ + zi * tileSize, z1 = z0 + tileSize;

                Point2D p0 = projectCamera(x0, y, z0, winW, winH);
                Point2D p1 = projectCamera(x1, y, z0, winW, winH);
                Point2D p2 = projectCamera(x1, y, z1, winW, winH);
                Point2D p3 = projectCamera(x0, y, z1, winW, winH);

                if (p0.z < 0.3f || p1.z < 0.3f || p2.z < 0.3f || p3.z < 0.3f) continue;

                float fog = mathMin(1.0f, p0.z / 26.0f);
                uint8_t lightR = static_cast<uint8_t>(mathClamp(200.0f - fog * 90.0f, 60.0f, 255.0f));
                uint8_t lightG = static_cast<uint8_t>(mathClamp(220.0f - fog * 80.0f, 80.0f, 255.0f));
                uint8_t lightB = static_cast<uint8_t>(mathClamp(200.0f - fog * 90.0f, 60.0f, 255.0f));

                SDL_Color c = { lightR, lightG, lightB, 255 };

                float u0 = x0 * kUvScale, u1 = x1 * kUvScale;
                float v0 = z0 * kUvScale, v1 = z1 * kUvScale;

                batchScratch.push_back({{ p0.x, p0.y }, c, {u0, v0}});
                batchScratch.push_back({{ p1.x, p1.y }, c, {u1, v0}});
                batchScratch.push_back({{ p2.x, p2.y }, c, {u1, v1}});
                batchScratch.push_back({{ p0.x, p0.y }, c, {u0, v0}});
                batchScratch.push_back({{ p2.x, p2.y }, c, {u1, v1}});
                batchScratch.push_back({{ p3.x, p3.y }, c, {u0, v1}});
            }
        }

        if (!batchScratch.empty()) {
            SDL_RenderGeometry(renderer, groundTexture, batchScratch.data(),
                               static_cast<int>(batchScratch.size()), nullptr, 0);
        }
    }

    void renderPlayerShadow(SDL_Renderer* renderer, int winW, int winH, float pX, float pZ, float pY) {
        float radius = mathMax(0.2f, 0.65f - pY * 0.08f);
        const float y = 0.02f;
        const int segs = 14;
        SDL_Color sc = { 10, 20, 10, 140 };
        Point2D center = projectCamera(pX, y, pZ, winW, winH);
        if (center.z < 0.5f) return;

        batchScratch.clear();
        batchScratch.reserve(static_cast<size_t>(segs) * 3);
        for (int i = 0; i < segs; ++i) {
            float a1 = (i     * 6.28318f) / segs;
            float a2 = ((i+1) * 6.28318f) / segs;
            Point2D pt1 = projectCamera(pX + std::cos(a1) * radius, y, pZ + std::sin(a1) * radius, winW, winH);
            Point2D pt2 = projectCamera(pX + std::cos(a2) * radius, y, pZ + std::sin(a2) * radius, winW, winH);
            batchScratch.push_back({{ center.x, center.y }, sc, {0.5f, 0.5f}});
            batchScratch.push_back({{ pt1.x,    pt1.y    }, sc, {0.0f, 0.0f}});
            batchScratch.push_back({{ pt2.x,    pt2.y    }, sc, {1.0f, 1.0f}});
        }
        SDL_RenderGeometry(renderer, nullptr, batchScratch.data(),
                           static_cast<int>(batchScratch.size()), nullptr, 0);
    }

    void renderStaticMesh(SDL_Renderer* renderer, int winW, int winH,
                          const Model3D& mesh, SDL_Texture* tex) {
        if (mesh.vertices.empty() || mesh.triangles.empty()) return;

        projScratch.resize(mesh.vertices.size());
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            projScratch[i] = projectCamera(mesh.vertices[i].x,
                                           mesh.vertices[i].y,
                                           mesh.vertices[i].z, winW, winH);
        }

        drawScratch.clear();
        drawScratch.reserve(mesh.triangles.size());
        for (const auto& t : mesh.triangles) {
            if (t.v0 >= projScratch.size() || t.v1 >= projScratch.size() || t.v2 >= projScratch.size()) continue;
            if (projScratch[t.v0].z < 0.2f || projScratch[t.v1].z < 0.2f || projScratch[t.v2].z < 0.2f) continue;
            if (!isFrontFacing(projScratch[t.v0], projScratch[t.v1], projScratch[t.v2])) continue;
            float avgZ = (projScratch[t.v0].z + projScratch[t.v1].z + projScratch[t.v2].z) / 3.0f;
            drawScratch.push_back({ t.v0, t.v1, t.v2, avgZ });
        }
        std::sort(drawScratch.begin(), drawScratch.end(),
                  [](const RTri& a, const RTri& b){ return b.avgZ < a.avgZ; });

        batchScratch.clear();
        batchScratch.reserve(drawScratch.size() * 3);

        for (const auto& t : drawScratch) {
            const auto& v0 = mesh.vertices[t.v0];
            const auto& v1 = mesh.vertices[t.v1];
            const auto& v2 = mesh.vertices[t.v2];
            const auto& p0 = projScratch[t.v0];
            const auto& p1 = projScratch[t.v1];
            const auto& p2 = projScratch[t.v2];

            float nx, ny, nz;
            faceNormal(v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, nx, ny, nz);
            float L = mathMin(1.0f, computeLighting(nx, ny, nz, lightDirX, lightDirY, lightDirZ, ambientIntensity));
            float fog = mathMin(1.0f, t.avgZ / 20.0f);

            batchScratch.push_back({{ p0.x, p0.y },
                { shadeVal(v0.r,L,fog), shadeVal(v0.g,L,fog), shadeVal(v0.b,L,fog), 255 }, { v0.u, v0.v }});
            batchScratch.push_back({{ p1.x, p1.y },
                { shadeVal(v1.r,L,fog), shadeVal(v1.g,L,fog), shadeVal(v1.b,L,fog), 255 }, { v1.u, v1.v }});
            batchScratch.push_back({{ p2.x, p2.y },
                { shadeVal(v2.r,L,fog), shadeVal(v2.g,L,fog), shadeVal(v2.b,L,fog), 255 }, { v2.u, v2.v }});
        }

        if (!batchScratch.empty()) {
            SDL_RenderGeometry(renderer, tex, batchScratch.data(),
                               static_cast<int>(batchScratch.size()), nullptr, 0);
        }
    }

    // ─── CONKER CHARACTER SKELETAL ANIMATION DEFORMER ────────────────────────
    void renderConkerMesh3D(SDL_Renderer* renderer, int winW, int winH, const ActorState& player) {
        if (conkerMesh.vertices.empty() || conkerMesh.triangles.empty()) return;

        float radY = player.rotY * kDegToRad;
        float cosY = std::cos(radY), sinY = std::sin(radY);

        projScratch.resize(conkerMesh.vertices.size());
        worldScratch.resize(conkerMesh.vertices.size());

        float legRad  = player.legAngle      * kDegToRad;
        float armRad  = player.armAngle      * kDegToRad;
        float tailRad = player.tailAngle     * kDegToRad;
        float spinRad = player.tailSpinAngle * kDegToRad;
        float leanRad = player.bodyLean      * kDegToRad;

        for (size_t i = 0; i < conkerMesh.vertices.size(); ++i) {
            const auto& v = conkerMesh.vertices[i];
            float lx = v.x, ly = v.y, lz = v.z;

            // 1. Deformacion esqueletica procedural por zona anatomica
            if (v.y < -0.8f) {
                float sign = (v.x > 0.0f) ? 1.0f : -1.0f;
                float a = legRad * sign;
                float hipY = -0.8f, hipZ = 0.0f;
                float dy = ly - hipY, dz = lz - hipZ;
                ly = hipY + dy * std::cos(a) - dz * std::sin(a);
                lz = hipZ + dy * std::sin(a) + dz * std::cos(a);
            }
            else if (std::abs(v.x) > 0.45f && v.y > -0.7f && v.y < 0.2f) {
                float sign = (v.x > 0.0f) ? -1.0f : 1.0f;
                float a = armRad * sign;
                float shoulderY = -0.1f, shoulderZ = 0.0f;
                float dy = ly - shoulderY, dz = lz - shoulderZ;
                ly = shoulderY + dy * std::cos(a) - dz * std::sin(a);
                lz = shoulderZ + dy * std::sin(a) + dz * std::cos(a);
            }
            else if (v.z < -0.24f && v.y > -0.60f) {
                // Rotación unificada de toda la cola desde la base
                float tailPivotX = 0.0f, tailPivotY = -0.35f, tailPivotZ = -0.28f;
                if (player.animState == AnimState::HOVER || player.animState == AnimState::ATTACK) {
                    float tx = lx - tailPivotX, ty = ly - tailPivotY;
                    lx = tailPivotX + tx * std::cos(spinRad) - ty * std::sin(spinRad);
                    ly = tailPivotY + tx * std::sin(spinRad) + ty * std::cos(spinRad);
                } else {
                    float dx = lx - tailPivotX, dz = lz - tailPivotZ;
                    lx = tailPivotX + dx * std::cos(tailRad) - dz * std::sin(tailRad);
                    lz = tailPivotZ + dx * std::sin(tailRad) + dz * std::cos(tailRad);
                }
            }
            else if (v.y > 0.3f) {
                ly += player.bobY * 0.5f;
            }

            if (leanRad > 0.01f && v.y > -0.8f) {
                float dy = ly, dz = lz;
                ly = dy * std::cos(leanRad) - dz * std::sin(leanRad);
                lz = dy * std::sin(leanRad) + dz * std::cos(leanRad);
            }
            ly += player.bobY;

            // 2. Modelo -> mundo
            float rx =  lx * cosY + lz * sinY;
            float rz = -lx * sinY + lz * cosY;
            float wx = rx + player.posX;
            float wy = ly + player.posY;
            float wz = rz + player.posZ;

            worldScratch[i] = { wx, wy, wz };
            projScratch[i]  = projectCamera(wx, wy, wz, winW, winH);
        }

        drawScratch.clear();
        drawScratch.reserve(conkerMesh.triangles.size());
        for (const auto& t : conkerMesh.triangles) {
            if (t.v0 >= projScratch.size() || t.v1 >= projScratch.size() || t.v2 >= projScratch.size()) continue;
            if (projScratch[t.v0].z < 0.2f || projScratch[t.v1].z < 0.2f || projScratch[t.v2].z < 0.2f) continue;
            if (!isFrontFacing(projScratch[t.v0], projScratch[t.v1], projScratch[t.v2])) continue;
            float avgZ = (projScratch[t.v0].z + projScratch[t.v1].z + projScratch[t.v2].z) / 3.0f;
            drawScratch.push_back({ t.v0, t.v1, t.v2, avgZ });
        }
        std::sort(drawScratch.begin(), drawScratch.end(),
                  [](const RTri& a, const RTri& b){ return b.avgZ < a.avgZ; });

        batchScratch.clear();
        batchScratch.reserve(drawScratch.size() * 3);

        for (const auto& t : drawScratch) {
            const auto& v0 = conkerMesh.vertices[t.v0];
            const auto& v1 = conkerMesh.vertices[t.v1];
            const auto& v2 = conkerMesh.vertices[t.v2];
            const auto& w0 = worldScratch[t.v0];
            const auto& w1 = worldScratch[t.v1];
            const auto& w2 = worldScratch[t.v2];
            const auto& p0 = projScratch[t.v0];
            const auto& p1 = projScratch[t.v1];
            const auto& p2 = projScratch[t.v2];

            float nx, ny, nz;
            faceNormal(w0[0], w0[1], w0[2], w1[0], w1[1], w1[2], w2[0], w2[1], w2[2], nx, ny, nz);
            float L = mathMin(1.0f, computeLighting(nx, ny, nz, lightDirX, lightDirY, lightDirZ, ambientIntensity));
            float fog = mathMin(1.0f, t.avgZ / 20.0f);

            batchScratch.push_back({{ p0.x, p0.y },
                { shadeVal(v0.r,L,fog), shadeVal(v0.g,L,fog), shadeVal(v0.b,L,fog), 255 }, { v0.u, v0.v }});
            batchScratch.push_back({{ p1.x, p1.y },
                { shadeVal(v1.r,L,fog), shadeVal(v1.g,L,fog), shadeVal(v1.b,L,fog), 255 }, { v1.u, v1.v }});
            batchScratch.push_back({{ p2.x, p2.y },
                { shadeVal(v2.r,L,fog), shadeVal(v2.g,L,fog), shadeVal(v2.b,L,fog), 255 }, { v2.u, v2.v }});
        }

        if (!batchScratch.empty()) {
            SDL_RenderGeometry(renderer, conkerMesh.sdlTexture,
                               batchScratch.data(), static_cast<int>(batchScratch.size()), nullptr, 0);
        }
    }
};

} // namespace N64
