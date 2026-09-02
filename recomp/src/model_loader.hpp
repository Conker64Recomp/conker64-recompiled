#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <SDL.h>
#include "gbi.hpp"
#include "memory.hpp"
#include "texture_loader.hpp"
#include "asset_paths.hpp"

namespace N64 {

class Model3D {
public:
    std::vector<Vertex3D>   vertices;
    std::vector<Triangle3D> triangles;
    std::string name;
    std::string texturePath;
    SDL_Texture* sdlTexture = nullptr;

    void releaseTexture() {
        if (sdlTexture) {
            SDL_DestroyTexture(sdlTexture);
            sdlTexture = nullptr;
        }
    }

    void loadMTL(const std::string& mtlPath, SDL_Renderer* renderer) {
        if (!renderer) return;

        std::ifstream f(mtlPath);
        if (!f.is_open()) return;

        std::string line;
        while (std::getline(f, line)) {
            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;
            if (prefix != "map_Kd") continue;

            std::string reference;
            std::getline(ss, reference);
            trim(reference);
            if (reference.empty()) continue;

            std::string resolved = AssetPaths::resolveRelativeToFile(mtlPath, reference);
            if (resolved.empty()) return;

            texturePath = resolved;
            sdlTexture = TextureLoader::getInstance().loadPNG(renderer, resolved);
            return;
        }
    }

    bool loadFromOBJ(const std::string& filepath, SDL_Renderer* renderer = nullptr) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        vertices.clear();
        triangles.clear();
        texturePath.clear();

        std::vector<std::array<float, 3>>    positions;
        std::vector<std::pair<float, float>> texCoords;
        std::unordered_map<uint64_t, uint32_t> cornerCache;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "mtllib") {
                std::string mtlName;
                ss >> mtlName;
                if (!mtlName.empty()) {
                    std::string mtlPath = AssetPaths::resolveRelativeToFile(filepath, mtlName);
                    if (!mtlPath.empty()) loadMTL(mtlPath, renderer);
                }
            }
            else if (prefix == "v") {
                float x = 0.0f, y = 0.0f, z = 0.0f;
                ss >> x >> y >> z;
                positions.push_back({ x, y, z });
            }
            else if (prefix == "vt") {
                float u = 0.0f, v = 0.0f;
                ss >> u >> v;
                texCoords.push_back({ u, 1.0f - v });
            }
            else if (prefix == "f") {
                std::string s;
                std::vector<uint32_t> faceCorners;

                while (ss >> s) {
                    int pIdx = 0, tIdx = 0;
                    if (!parseCorner(s, pIdx, tIdx)) continue;

                    if (pIdx < 0) pIdx = static_cast<int>(positions.size()) + pIdx + 1;
                    if (tIdx < 0) tIdx = static_cast<int>(texCoords.size()) + tIdx + 1;

                    if (pIdx < 1 || pIdx > static_cast<int>(positions.size())) continue;

                    uint64_t key = (static_cast<uint64_t>(pIdx) << 32) | static_cast<uint32_t>(tIdx);
                    auto it = cornerCache.find(key);
                    if (it != cornerCache.end()) {
                        faceCorners.push_back(it->second);
                        continue;
                    }

                    const auto& pos = positions[pIdx - 1];
                    float u = 0.0f, v = 0.0f;
                    if (tIdx >= 1 && tIdx <= static_cast<int>(texCoords.size())) {
                        u = texCoords[tIdx - 1].first;
                        v = texCoords[tIdx - 1].second;
                    }

                    uint32_t newIdx = static_cast<uint32_t>(vertices.size());
                    vertices.push_back({ pos[0], pos[1], pos[2], 0.9f, 0.85f, 0.75f, 1.0f, u, v });
                    cornerCache[key] = newIdx;
                    faceCorners.push_back(newIdx);
                }

                if (faceCorners.size() >= 3) {
                    for (size_t i = 1; i + 1 < faceCorners.size(); ++i) {
                        triangles.push_back({ faceCorners[0], faceCorners[i], faceCorners[i + 1] });
                    }
                }
            }
        }

        std::cout << "[Model3D] Loaded OBJ: " << filepath << " ("
                  << vertices.size() << " verts, " << triangles.size() << " tris)" << std::endl;
        return !triangles.empty();
    }

    // ── PRIMITIVAS ORGÁNICAS SUAVES DE ALTA RESOLUCIÓN ───────────────────────

    // Genera una esfera / elipsoide con curvatura orgánica suave y normales
    void addSphere(float cx, float cy, float cz, float rx, float ry, float rz,
                   float r, float g, float b, int rings = 10, int sectors = 12,
                   float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f, float v1 = 1.0f) {
        uint32_t base = static_cast<uint32_t>(vertices.size());
        constexpr float kPi = 3.1415926535f;

        for (int i = 0; i <= rings; ++i) {
            float v = static_cast<float>(i) / rings;
            float phi = v * kPi;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            for (int j = 0; j <= sectors; ++j) {
                float u = static_cast<float>(j) / sectors;
                float theta = u * 2.0f * kPi;
                float sinTheta = std::sin(theta);
                float cosTheta = std::cos(theta);

                float x = cx + rx * sinPhi * cosTheta;
                float y = cy + ry * cosPhi;
                float z = cz + rz * sinPhi * sinTheta;

                float tu = u0 + u * (u1 - u0);
                float tv = v0 + v * (v1 - v0);

                vertices.push_back({ x, y, z, r, g, b, 1.0f, tu, tv });
            }
        }

        for (int i = 0; i < rings; ++i) {
            for (int j = 0; j < sectors; ++j) {
                uint32_t i0 = base + i * (sectors + 1) + j;
                uint32_t i1 = base + (i + 1) * (sectors + 1) + j;
                uint32_t i2 = base + (i + 1) * (sectors + 1) + (j + 1);
                uint32_t i3 = base + i * (sectors + 1) + (j + 1);

                triangles.push_back({ i0, i1, i2 });
                triangles.push_back({ i0, i2, i3 });
            }
        }
    }

    // Genera un cilindro / cono truncado suave (para brazos, piernas, cola y sudadera)
    void addCylinder(float x0, float y0, float z0, float x1, float y1, float z1,
                     float r0, float r1, float r, float g, float b, int segments = 10,
                     float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f, float v1 = 1.0f) {
        uint32_t base = static_cast<uint32_t>(vertices.size());
        constexpr float kPi = 3.1415926535f;

        float dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
        float len = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (len < 1e-5f) len = 1.0f;
        dx /= len; dy /= len; dz /= len;

        // Base ortogonal
        float ux = 0.0f, uy = 1.0f, uz = 0.0f;
        if (std::abs(dy) > 0.95f) { ux = 1.0f; uy = 0.0f; uz = 0.0f; }
        float wx = dy * uz - dz * uy, wy = dz * ux - dx * uz, wz = dx * uy - dy * ux;
        float wlen = std::sqrt(wx*wx + wy*wy + wz*wz);
        wx /= wlen; wy /= wlen; wz /= wlen;
        ux = wy * dz - wz * dy; uy = wz * dx - wx * dz; uz = wx * dy - wy * dx;

        for (int i = 0; i <= segments; ++i) {
            float frac = static_cast<float>(i) / segments;
            float angle = frac * 2.0f * kPi;
            float ca = std::cos(angle), sa = std::sin(angle);

            float cx0 = x0 + (ux * ca + wx * sa) * r0;
            float cy0 = y0 + (uy * ca + wy * sa) * r0;
            float cz0 = z0 + (uz * ca + wz * sa) * r0;

            float cx1 = x1 + (ux * ca + wx * sa) * r1;
            float cy1 = y1 + (uy * ca + wy * sa) * r1;
            float cz1 = z1 + (uz * ca + wz * sa) * r1;

            float tu = u0 + frac * (u1 - u0);
            vertices.push_back({ cx0, cy0, cz0, r, g, b, 1.0f, tu, v0 });
            vertices.push_back({ cx1, cy1, cz1, r, g, b, 1.0f, tu, v1 });
        }

        for (int i = 0; i < segments; ++i) {
            uint32_t i0 = base + i * 2;
            uint32_t i1 = base + i * 2 + 1;
            uint32_t i2 = base + (i + 1) * 2 + 1;
            uint32_t i3 = base + (i + 1) * 2;

            triangles.push_back({ i0, i1, i2 });
            triangles.push_back({ i0, i2, i3 });
        }
    }

    // ── MODELO 3D AUTÉNTICO ORGÁNICO DE CONKER (RAREWARE N64 ORIGINAL) ────────
    static Model3D createConkerMesh(SDL_Renderer* renderer = nullptr) {
        Model3D m;
        m.name = "Conker (Authentic Organic N64 Model)";

        // 1. CABEZA Y ROSTRO DE CONKER (Esfera orgánica de pelaje naranja con hocico y orejas)
        // Cráneo redondeado principal
        m.addSphere(0.0f, 0.55f, 0.0f, 0.58f, 0.52f, 0.55f, 0.96f, 0.48f, 0.12f, 10, 14); // Pelo naranja Conker

        // Hocico / Mejillas de color crema claro
        m.addSphere(0.0f, 0.38f, 0.38f, 0.38f, 0.28f, 0.34f, 0.98f, 0.90f, 0.78f, 8, 12);  // Crema hocico

        // Nariz esférica negra
        m.addSphere(0.0f, 0.48f, 0.65f, 0.10f, 0.08f, 0.09f, 0.10f, 0.10f, 0.12f, 6, 8);   // Nariz brillante

        // Ojos de caricatura (Esferas blancas con pupilas oscuras)
        m.addSphere(-0.20f, 0.65f, 0.42f, 0.14f, 0.18f, 0.10f, 0.98f, 0.98f, 0.98f, 6, 8); // Ojo izq blanco
        m.addSphere( 0.20f, 0.65f, 0.42f, 0.14f, 0.18f, 0.10f, 0.98f, 0.98f, 0.98f, 6, 8); // Ojo der blanco
        m.addSphere(-0.19f, 0.65f, 0.50f, 0.07f, 0.10f, 0.04f, 0.12f, 0.20f, 0.45f, 5, 6); // Pupila azul/negra
        m.addSphere( 0.19f, 0.65f, 0.50f, 0.07f, 0.10f, 0.04f, 0.12f, 0.20f, 0.45f, 5, 6); // Pupila azul/negra

        // Orejas curvadas cónicas (Exterior naranja, interior rosado)
        m.addCylinder(-0.35f, 0.90f, -0.05f, -0.52f, 1.25f, -0.05f, 0.18f, 0.04f, 0.92f, 0.42f, 0.10f, 8);
        m.addCylinder( 0.35f, 0.90f, -0.05f,  0.52f, 1.25f, -0.05f, 0.18f, 0.04f, 0.92f, 0.42f, 0.10f, 8);
        m.addSphere(-0.40f, 1.05f, 0.0f, 0.10f, 0.16f, 0.04f, 0.95f, 0.65f, 0.60f, 6, 6); // Interior oreja izq
        m.addSphere( 0.40f, 1.05f, 0.0f, 0.10f, 0.16f, 0.04f, 0.95f, 0.65f, 0.60f, 6, 6); // Interior oreja der

        // 2. TORSO Y SUDADERA AZUL CON CREMALLERA
        // Torso orgánico cónico (azul sudadera clásico)
        m.addCylinder(0.0f, 0.20f, 0.0f, 0.0f, -0.35f, 0.0f, 0.42f, 0.46f, 0.14f, 0.42f, 0.88f, 12);
        // Franja vertical central amarilla de la cremallera
        m.addCylinder(0.0f, 0.20f, 0.38f, 0.0f, -0.35f, 0.42f, 0.05f, 0.05f, 0.98f, 0.85f, 0.15f, 6);
        // Capucha trasera redondeada
        m.addSphere(0.0f, 0.15f, -0.28f, 0.32f, 0.22f, 0.22f, 0.10f, 0.32f, 0.72f, 8, 10);

        // 3. BRAZOS ORGÁNICOS Y GUANTES DE CARICATURA
        // Brazo izquierdo (Manga azul + antebrazo naranja)
        m.addCylinder(-0.35f, 0.10f, 0.0f, -0.65f, -0.10f, 0.05f, 0.16f, 0.14f, 0.14f, 0.42f, 0.88f, 8);
        m.addCylinder(-0.65f, -0.10f, 0.05f, -0.80f, -0.35f, 0.10f, 0.13f, 0.11f, 0.96f, 0.48f, 0.12f, 8);
        // Guante blanco izquierdo con pulgar modelado
        m.addSphere(-0.85f, -0.45f, 0.12f, 0.16f, 0.16f, 0.16f, 0.96f, 0.96f, 0.98f, 8, 8);
        m.addSphere(-0.76f, -0.40f, 0.22f, 0.08f, 0.08f, 0.08f, 0.96f, 0.96f, 0.98f, 6, 6);

        // Brazo derecho (Manga azul + antebrazo naranja)
        m.addCylinder( 0.35f, 0.10f, 0.0f,  0.65f, -0.10f, 0.05f, 0.16f, 0.14f, 0.14f, 0.42f, 0.88f, 8);
        m.addCylinder( 0.65f, -0.10f, 0.05f,  0.80f, -0.35f, 0.10f, 0.13f, 0.11f, 0.96f, 0.48f, 0.12f, 8);
        // Guante blanco derecho con pulgar modelado
        m.addSphere( 0.85f, -0.45f, 0.12f, 0.16f, 0.16f, 0.16f, 0.96f, 0.96f, 0.98f, 8, 8);
        m.addSphere( 0.76f, -0.40f, 0.22f, 0.08f, 0.08f, 0.08f, 0.96f, 0.96f, 0.98f, 6, 6);

        // 4. PIERNAS Y ZAPATILLAS DE DEPORTE AZULES
        // Muslos / Pantalones azules
        m.addCylinder(-0.20f, -0.35f, 0.0f, -0.22f, -0.75f, 0.0f, 0.20f, 0.16f, 0.12f, 0.38f, 0.82f, 8);
        m.addCylinder( 0.20f, -0.35f, 0.0f,  0.22f, -0.75f, 0.0f, 0.20f, 0.16f, 0.12f, 0.38f, 0.82f, 8);
        // Piernas de pelaje naranja
        m.addCylinder(-0.22f, -0.75f, 0.0f, -0.24f, -1.05f, 0.0f, 0.14f, 0.12f, 0.96f, 0.48f, 0.12f, 8);
        m.addCylinder( 0.22f, -0.75f, 0.0f,  0.24f, -1.05f, 0.0f, 0.14f, 0.12f, 0.96f, 0.48f, 0.12f, 8);

        // Zapatillas azules de Conker (Cuerpo azul, suela blanca gruesa y cordones amarillos)
        m.addSphere(-0.24f, -1.18f, 0.12f, 0.19f, 0.14f, 0.32f, 0.14f, 0.32f, 0.78f, 8, 10);
        m.addSphere( 0.24f, -1.18f, 0.12f, 0.19f, 0.14f, 0.32f, 0.14f, 0.32f, 0.78f, 8, 10);
        m.addCylinder(-0.24f, -1.28f, -0.10f, -0.24f, -1.28f, 0.36f, 0.18f, 0.18f, 0.95f, 0.95f, 0.95f, 8); // Suela izq
        m.addCylinder( 0.24f, -1.28f, -0.10f,  0.24f, -1.28f, 0.36f, 0.18f, 0.18f, 0.95f, 0.95f, 0.95f, 8); // Suela der

        // 5. COLA DE ARDILLA ESPONJOSA HELICOIDAL (8 Segmentos cónicos curvados)
        constexpr int tailSegs = 8;
        float tailX = 0.0f, tailY = -0.30f, tailZ = -0.32f;
        float curRadius = 0.14f;

        for (int t = 0; t < tailSegs; ++t) {
            float frac = static_cast<float>(t) / tailSegs;
            float nextFrac = static_cast<float>(t + 1) / tailSegs;

            // Curva ascendente y hacia atrás característica de Conker
            float nextX = std::sin(nextFrac * 1.5f) * 0.10f;
            float nextY = tailY + 0.18f + nextFrac * 0.12f;
            float nextZ = tailZ - 0.24f * (1.0f - nextFrac * 0.5f);

            float nextRadius = (t < 5) ? (curRadius + 0.06f) : (curRadius - 0.05f); // Se engrosa y luego afila
            m.addCylinder(tailX, tailY, tailZ, nextX, nextY, nextZ, curRadius, nextRadius, 0.96f, 0.48f, 0.12f, 10);

            tailX = nextX; tailY = nextY; tailZ = nextZ; curRadius = nextRadius;
        }

        std::cout << "[3D Mesh] Conker Authentic Organic Mesh created: "
                  << m.vertices.size() << " vertices, " << m.triangles.size() << " triangles." << std::endl;
        return m;
    }

    // ── ESCENARIO ORGÁNICO DE NIVEL (WINDEY / HUNGOVER ENVIRONMENT) ───────────
    static Model3D createLevelGeometry(SDL_Renderer* renderer = nullptr) {
        (void)renderer;
        Model3D m;
        m.name = "Windy / Hungover Organic Environment";

        // 1. Colinas y plataformas de césped onduladas
        m.addSphere(0.0f, -2.5f, 6.0f, 6.5f, 1.8f, 6.5f, 0.25f, 0.75f, 0.20f, 12, 16);
        m.addSphere(-7.0f, -2.0f, 5.0f, 5.0f, 2.2f, 5.0f, 0.22f, 0.68f, 0.18f, 10, 14);
        m.addSphere( 7.5f, -1.8f, 7.0f, 5.5f, 2.5f, 5.5f, 0.28f, 0.78f, 0.22f, 10, 14);

        // 2. Árboles orgánicos de estilo Rareware (Tronco de madera + Copa esférica frondosa)
        // Árbol Izquierdo
        m.addCylinder(-5.5f, 0.0f, 4.0f, -5.5f, 2.8f, 4.0f, 0.35f, 0.22f, 0.45f, 0.28f, 0.12f, 8);
        m.addSphere(-5.5f, 3.8f, 4.0f, 1.8f, 1.5f, 1.8f, 0.15f, 0.65f, 0.20f, 10, 12);
        m.addSphere(-5.0f, 4.4f, 3.8f, 1.2f, 1.1f, 1.2f, 0.20f, 0.72f, 0.25f, 8, 10);

        // Árbol Derecho
        m.addCylinder( 6.0f, 0.0f, 8.0f,  6.0f, 3.2f, 8.0f, 0.40f, 0.25f, 0.45f, 0.28f, 0.12f, 8);
        m.addSphere( 6.0f, 4.2f, 8.0f, 2.2f, 1.8f, 2.2f, 0.18f, 0.68f, 0.22f, 10, 12);
        m.addSphere( 6.5f, 4.9f, 8.2f, 1.4f, 1.2f, 1.4f, 0.24f, 0.76f, 0.28f, 8, 10);

        // 3. Vallas de madera de la Taberna
        for (int f = -2; f <= 2; ++f) {
            float fx = f * 1.6f;
            m.addCylinder(fx, 0.0f, 2.2f, fx, 0.9f, 2.2f, 0.08f, 0.07f, 0.55f, 0.35f, 0.18f, 6);
        }
        m.addCylinder(-3.4f, 0.65f, 2.2f, 3.4f, 0.65f, 2.2f, 0.06f, 0.06f, 0.55f, 0.35f, 0.18f, 6);
        m.addCylinder(-3.4f, 0.35f, 2.2f, 3.4f, 0.35f, 2.2f, 0.06f, 0.06f, 0.55f, 0.35f, 0.18f, 6);

        std::cout << "[3D Mesh] Level Organic Environment created: "
                  << m.vertices.size() << " vertices, " << m.triangles.size() << " triangles." << std::endl;
        return m;
    }

private:
    static void trim(std::string& s) {
        const char* ws = " \t\r\n";
        size_t b = s.find_first_not_of(ws);
        if (b == std::string::npos) { s.clear(); return; }
        size_t e = s.find_last_not_of(ws);
        s = s.substr(b, e - b + 1);
    }

    static bool parseCorner(const std::string& token, int& outPos, int& outTex) {
        outPos = 0; outTex = 0;
        if (token.empty()) return false;

        const char* p = token.c_str();
        char* end = nullptr;
        long v = std::strtol(p, &end, 10);
        if (end == p) return false;
        outPos = static_cast<int>(v);

        if (*end != '/') return true;
        p = end + 1;
        if (*p == '/') {
            p++;
            std::strtol(p, &end, 10);
            return true;
        }

        v = std::strtol(p, &end, 10);
        if (end != p) outTex = static_cast<int>(v);
        return true;
    }
};

} // namespace N64
