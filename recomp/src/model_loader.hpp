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
    std::string texturePath;            // ruta resuelta de la textura (si la hay)
    SDL_Texture* sdlTexture = nullptr;

    void releaseTexture() {
        if (sdlTexture) {
            SDL_DestroyTexture(sdlTexture);
            sdlTexture = nullptr;
        }
    }

    // ── Carga el MTL emparejado con el OBJ ────────────────────────────────
    //
    // `map_Kd` es relativo al directorio del propio MTL (semantica del formato).
    // Resolverlo contra el cwd era la razon por la que no cargaba ni una sola
    // textura de los 749 modelos.
    void loadMTL(const std::string& mtlPath, SDL_Renderer* renderer) {
        if (!renderer) return;

        std::ifstream f(mtlPath);
        if (!f.is_open()) {
            std::cerr << "[Model3D] MTL not found: " << mtlPath << std::endl;
            return;
        }

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
            if (resolved.empty()) {
                std::cerr << "[Model3D] Texture referenced by " << mtlPath
                          << " not found: " << reference << std::endl;
                return;
            }

            texturePath = resolved;
            sdlTexture = TextureLoader::getInstance().loadPNG(renderer, resolved);
            if (sdlTexture) {
                std::cout << "[Model3D] Texture bound: " << reference << std::endl;
            }
            return;
        }
    }

    // ── Wavefront OBJ (+ MTL opcional) ────────────────────────────────────
    bool loadFromOBJ(const std::string& filepath, SDL_Renderer* renderer = nullptr) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        vertices.clear();
        triangles.clear();
        texturePath.clear();

        std::vector<std::array<float, 3>>    positions;
        std::vector<std::pair<float, float>> texCoords;

        // Una posicion compartida por caras con UV distintas necesita vertices
        // distintos; si no, la ultima cara sobrescribe las UV de las anteriores.
        std::unordered_map<uint64_t, uint32_t> cornerCache;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "mtllib") {
                std::string mtlFile;
                ss >> mtlFile;
                if (!mtlFile.empty()) {
                    std::string resolved = AssetPaths::resolveRelativeToFile(filepath, mtlFile);
                    if (!resolved.empty()) loadMTL(resolved, renderer);
                }
            }
            else if (prefix == "v") {
                float x = 0, y = 0, z = 0;
                ss >> x >> y >> z;
                positions.push_back({ x, y, z });
            }
            else if (prefix == "vt") {
                float u = 0, v = 0;
                ss >> u >> v;
                texCoords.push_back({ u, v });
            }
            else if (prefix == "f") {
                std::vector<uint32_t> corners;
                std::string token;
                while (ss >> token) {
                    int rawPos = 0, rawTex = 0;
                    if (!parseCorner(token, rawPos, rawTex)) continue;

                    int p = resolveIndex(rawPos, positions.size());
                    if (p < 0 || p >= static_cast<int>(positions.size())) continue;

                    int t = -1;
                    if (rawTex != 0) {
                        t = resolveIndex(rawTex, texCoords.size());
                        if (t < 0 || t >= static_cast<int>(texCoords.size())) t = -1;
                    }

                    corners.push_back(emitCorner(positions, texCoords, cornerCache, p, t));
                }

                // Triangulacion en abanico: soporta triangulos, quads y n-gonos.
                for (size_t i = 2; i < corners.size(); ++i) {
                    triangles.push_back({ corners[0], corners[i - 1], corners[i] });
                }
            }
        }

        return !vertices.empty() && !triangles.empty();
    }

    // ── Caja procedural ───────────────────────────────────────────────────
    void addBox(float cx, float cy, float cz, float sx, float sy, float sz,
                float r, float g, float b, float u0, float v0, float u1, float v1) {
        uint32_t base = static_cast<uint32_t>(vertices.size());
        float hx = sx * .5f, hy = sy * .5f, hz = sz * .5f;
        float corners[8][3] = {
            {cx-hx,cy-hy,cz-hz},{cx+hx,cy-hy,cz-hz},{cx+hx,cy+hy,cz-hz},{cx-hx,cy+hy,cz-hz},
            {cx-hx,cy-hy,cz+hz},{cx+hx,cy-hy,cz+hz},{cx+hx,cy+hy,cz+hz},{cx-hx,cy+hy,cz+hz}
        };
        for (int i = 0; i < 8; ++i) {
            float u = (i % 2 == 0) ? u0 : u1;
            float v = (i / 2 % 2 == 0) ? v0 : v1;
            vertices.push_back({ corners[i][0], corners[i][1], corners[i][2], r, g, b, 1.0f, u, v });
        }
        int faces[6][4] = {{0,1,2,3},{5,4,7,6},{3,2,6,7},{4,5,1,0},{1,5,6,2},{4,0,3,7}};
        for (int f = 0; f < 6; ++f) {
            triangles.push_back({ base + faces[f][0], base + faces[f][1], base + faces[f][2] });
            triangles.push_back({ base + faces[f][0], base + faces[f][2], base + faces[f][3] });
        }
    }

    // ── Malla del personaje (OBJ de la ROM, con fallback procedural) ──────
    static Model3D createConkerMesh(SDL_Renderer* renderer = nullptr) {
        Model3D m;
        m.name = "Conker (Player Mesh)";

        // `conker_character.obj` que produce el extractor son 4 triangulos
        // sueltos, no un personaje. Por debajo del umbral se usa el modelo
        // procedural, que al menos es reconocible en pantalla.
        constexpr size_t kMinCharacterTriangles = 16;

        std::string path = AssetPaths::getInstance().resolve("models/conker_character.obj");
        if (!path.empty() && m.loadFromOBJ(path, renderer) &&
            m.triangles.size() >= kMinCharacterTriangles) {
            std::cout << "[3D] Conker ROM mesh: " << m.vertices.size()
                      << " verts, " << m.triangles.size() << " tris" << std::endl;
            return m;
        }

        std::cout << "[3D] Conker ROM mesh unusable (" << m.triangles.size()
                  << " tris); using procedural character." << std::endl;

        // Descartar la carga parcial: addBox indexa desde vertices.size().
        m.releaseTexture();
        m.vertices.clear();
        m.triangles.clear();
        m.name = "Conker (Procedural)";
        m.addBox(0.0f, 0.5f,  0.0f,  1.1f, 1.0f,1.0f,  0.95f,0.50f,0.15f, 0.0f,0.0f,1.0f,1.0f);
        m.addBox(0.0f, 0.3f,  0.55f, 0.7f, 0.45f,0.35f, 0.98f,0.90f,0.75f, 0.1f,0.1f,0.9f,0.9f);
        m.addBox(0.0f, 0.45f, 0.75f, 0.22f,0.18f,0.18f, 0.10f,0.10f,0.10f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox(-0.55f,1.15f,0.0f,  0.30f,0.50f,0.25f, 0.90f,0.40f,0.10f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox( 0.55f,1.15f,0.0f,  0.30f,0.50f,0.25f, 0.90f,0.40f,0.10f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox(0.0f, -0.4f, 0.0f,  0.9f, 0.85f,0.7f,  0.10f,0.40f,0.90f, 0.0f,0.0f,1.0f,1.0f);
        m.addBox(0.0f, -0.4f, 0.36f, 0.12f,0.80f,0.05f, 1.0f, 0.85f,0.10f, 0.0f,0.0f,0.2f,0.2f);
        m.addBox(-0.65f,-0.35f,0.0f, 0.35f,0.65f,0.35f, 0.10f,0.35f,0.85f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox( 0.65f,-0.35f,0.0f, 0.35f,0.65f,0.35f, 0.10f,0.35f,0.85f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox(-0.30f,-1.05f,0.0f, 0.32f,0.50f,0.35f, 0.90f,0.45f,0.15f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox( 0.30f,-1.05f,0.0f, 0.32f,0.50f,0.35f, 0.90f,0.45f,0.15f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox(-0.30f,-1.35f,0.15f,0.38f,0.25f,0.65f, 0.15f,0.45f,0.95f, 0.0f,0.0f,1.0f,1.0f);
        m.addBox( 0.30f,-1.35f,0.15f,0.38f,0.25f,0.65f, 0.15f,0.45f,0.95f, 0.0f,0.0f,1.0f,1.0f);
        m.addBox(0.0f,-0.3f,-0.65f,  0.45f,0.95f,0.55f, 0.92f,0.48f,0.12f, 0.0f,0.0f,1.0f,1.0f);
        return m;
    }

    // ── Geometria de nivel procedural (fallback) ──────────────────────────
    static Model3D createLevelGeometry(SDL_Renderer* renderer = nullptr) {
        (void)renderer;
        Model3D m;
        m.name = "Level Environment (Procedural)";
        m.addBox(0.0f,-1.25f,5.0f, 2.8f,0.20f,2.8f, 0.85f,0.25f,0.15f, 0.0f,0.0f,1.0f,1.0f);
        m.addBox(0.0f,-1.13f,5.0f, 1.8f,0.10f,1.8f, 1.00f,0.80f,0.10f, 0.2f,0.2f,0.8f,0.8f);
        m.addBox(-5.5f,-0.5f,4.0f, 3.5f,1.5f,3.5f,  0.55f,0.35f,0.20f, 0.0f,0.0f,1.0f,1.0f);
        m.addBox( 5.5f,-0.2f,4.0f, 3.5f,2.0f,3.5f,  0.45f,0.45f,0.45f, 0.0f,0.0f,1.0f,1.0f);
        m.addBox(-5.5f, 0.75f,4.0f,1.2f,1.5f,1.2f,  0.60f,0.30f,0.10f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox( 6.0f, 1.2f,8.0f, 0.6f,3.0f,0.6f,  0.40f,0.20f,0.10f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox( 6.0f, 3.0f,8.0f, 2.5f,1.5f,2.5f,  0.15f,0.65f,0.20f, 0.0f,0.0f,1.0f,1.0f);
        m.addBox(-6.0f, 1.2f,8.0f, 0.6f,3.0f,0.6f,  0.40f,0.20f,0.10f, 0.0f,0.0f,0.5f,0.5f);
        m.addBox(-6.0f, 3.0f,8.0f, 2.5f,1.5f,2.5f,  0.15f,0.65f,0.20f, 0.0f,0.0f,1.0f,1.0f);
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

    // "12", "12/7", "12/7/3", "12//3" -> indices crudos (0 = ausente).
    // strtol en vez de stoi: los OBJ del extractor traen tokens malformados y
    // una excepcion sin capturar tumbaba la carga entera del modelo.
    static bool parseCorner(const std::string& token, int& outPos, int& outTex) {
        outPos = 0;
        outTex = 0;
        if (token.empty()) return false;

        const char* p = token.c_str();
        char* end = nullptr;

        long v = std::strtol(p, &end, 10);
        if (end == p) return false;
        outPos = static_cast<int>(v);

        if (*end != '/') return true;
        p = end + 1;
        if (*p == '/') return true;   // formato "v//vn": sin coordenada de textura

        v = std::strtol(p, &end, 10);
        if (end != p) outTex = static_cast<int>(v);
        return true;
    }

    // OBJ indexa desde 1; los negativos son relativos al final de la lista.
    static int resolveIndex(int raw, size_t count) {
        if (raw > 0) return raw - 1;
        if (raw < 0) return static_cast<int>(count) + raw;
        return -1;
    }

    // Devuelve el indice del vertice (posicion, UV), creandolo si es la primera vez.
    uint32_t emitCorner(const std::vector<std::array<float, 3>>& positions,
                        const std::vector<std::pair<float, float>>& texCoords,
                        std::unordered_map<uint64_t, uint32_t>& cache,
                        int posIdx, int texIdx) {
        uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(posIdx)) << 32) |
                        static_cast<uint32_t>(texIdx);

        auto it = cache.find(key);
        if (it != cache.end()) return it->second;

        Vertex3D v{};
        v.x = positions[posIdx][0];
        v.y = positions[posIdx][1];
        v.z = positions[posIdx][2];
        v.r = v.g = v.b = v.a = 1.0f;
        v.u = 0.0f;
        v.v = 0.0f;
        if (texIdx >= 0) {
            v.u = texCoords[texIdx].first;
            v.v = texCoords[texIdx].second;
        }

        uint32_t index = static_cast<uint32_t>(vertices.size());
        vertices.push_back(v);
        cache.emplace(key, index);
        return index;
    }
};

// ── Procesador de Display Lists F3DEX2 ───────────────────────────────────
class DisplayListProcessor {
public:
    static DisplayListProcessor& getInstance() { static DisplayListProcessor i; return i; }

    void executeDL(uint32_t dlVaddr, Model3D& outMesh) {
        uint32_t paddr = Memory::getInstance().toPhysical(dlVaddr);
        if (paddr + sizeof(Gfx) > Memory::getInstance().getRDRAMSize()) return;

        std::vector<Vertex3D> vtxBuf(32);
        size_t cmd = 0;
        while (cmd < 2048) {
            uint32_t w0 = Memory::getInstance().read<uint32_t>(dlVaddr + cmd * 8);
            uint32_t w1 = Memory::getInstance().read<uint32_t>(dlVaddr + cmd * 8 + 4);
            ++cmd;
            uint8_t op = static_cast<uint8_t>(w0 >> 24);
            if (op == GBICommand::G_ENDDL) break;
            else if (op == GBICommand::G_VTX) {
                uint32_t n = (w0 >> 12) & 0xFF;
                for (uint32_t i = 0; i < n && i < 32; ++i) {
                    int16_t vx = Memory::getInstance().read<int16_t>(w1 + i * 16);
                    int16_t vy = Memory::getInstance().read<int16_t>(w1 + i * 16 + 2);
                    int16_t vz = Memory::getInstance().read<int16_t>(w1 + i * 16 + 4);
                    vtxBuf[i] = { vx / 100.f, vy / 100.f, vz / 100.f, 1,1,1,1, 0,0 };
                }
            }
            else if (op == GBICommand::G_TRI1) {
                uint8_t v0 = ((w0 >> 16) & 0xFF) / 2;
                uint8_t v1 = ((w0 >> 8) & 0xFF) / 2;
                uint8_t v2 = (w0 & 0xFF) / 2;
                uint32_t base = static_cast<uint32_t>(outMesh.vertices.size());
                outMesh.vertices.push_back(vtxBuf[v0]);
                outMesh.vertices.push_back(vtxBuf[v1]);
                outMesh.vertices.push_back(vtxBuf[v2]);
                outMesh.triangles.push_back({ base, base + 1, base + 2 });
            }
        }
    }
};

} // namespace N64
