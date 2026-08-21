#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
#include <SDL.h>
#include "gbi.hpp"
#include "memory.hpp"
#include "texture_loader.hpp"

namespace N64 {

class Model3D {
public:
    std::vector<Vertex3D>   vertices;
    std::vector<Triangle3D> triangles;
    std::string name;
    std::string texturePath;     // path to PNG texture (relative to executable)
    SDL_Texture* sdlTexture = nullptr;  // loaded at runtime

    // ── Load texture from MTL file paired with OBJ ────────────────────────
    void loadMTL(const std::string& mtlPath, SDL_Renderer* renderer) {
        if (!renderer) return;
        std::ifstream f(mtlPath);
        if (!f.is_open()) {
            // Try siblings
            std::string dir = mtlPath.substr(0, mtlPath.find_last_of("/\\") + 1);
            return;
        }
        std::string line;
        while (std::getline(f, line)) {
            if (line.size() > 7 && line.substr(0, 7) == "map_Kd ") {
                texturePath = line.substr(7);
                // Normalize slashes
                for (auto& c : texturePath) if (c == '\\') c = '/';
                sdlTexture = TextureLoader::getInstance().loadPNG(renderer, texturePath);
                if (sdlTexture)
                    std::cout << "[Model3D] Loaded texture: " << texturePath << std::endl;
                break;
            }
        }
    }

    // ── Load Wavefront OBJ (+ optional MTL) ──────────────────────────────
    bool loadFromOBJ(const std::string& filepath, SDL_Renderer* renderer = nullptr) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        vertices.clear();
        triangles.clear();
        texturePath.clear();

        std::vector<std::pair<float,float>> texCoords;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "mtllib") {
                std::string mtlFile;
                ss >> mtlFile;
                std::string dir = filepath.substr(0, filepath.find_last_of("/\\") + 1);
                loadMTL(dir + mtlFile, renderer);
            }
            else if (prefix == "v") {
                float x, y, z; ss >> x >> y >> z;
                vertices.push_back({x, y, z, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f});
            }
            else if (prefix == "vt") {
                float u, v; ss >> u >> v;
                texCoords.push_back({u, v});
            }
            else if (prefix == "f") {
                std::string s1, s2, s3;
                ss >> s1 >> s2 >> s3;
                auto parseIdx = [](const std::string& s) -> int {
                    size_t sl = s.find('/');
                    return std::stoi(sl == std::string::npos ? s : s.substr(0, sl)) - 1;
                };
                auto parseTex = [](const std::string& s) -> int {
                    size_t sl = s.find('/');
                    if (sl == std::string::npos) return -1;
                    size_t sl2 = s.find('/', sl+1);
                    std::string t = s.substr(sl+1, sl2 == std::string::npos ? std::string::npos : sl2-sl-1);
                    return t.empty() ? -1 : std::stoi(t) - 1;
                };
                int vi0=parseIdx(s1), vi1=parseIdx(s2), vi2=parseIdx(s3);
                int ti0=parseTex(s1), ti1=parseTex(s2), ti2=parseTex(s3);
                if (vi0<0||vi1<0||vi2<0) continue;
                if (vi0>=(int)vertices.size()||vi1>=(int)vertices.size()||vi2>=(int)vertices.size()) continue;
                // Assign UV
                if (ti0>=0 && ti0<(int)texCoords.size()) { vertices[vi0].u=texCoords[ti0].first; vertices[vi0].v=texCoords[ti0].second; }
                if (ti1>=0 && ti1<(int)texCoords.size()) { vertices[vi1].u=texCoords[ti1].first; vertices[vi1].v=texCoords[ti1].second; }
                if (ti2>=0 && ti2<(int)texCoords.size()) { vertices[vi2].u=texCoords[ti2].first; vertices[vi2].v=texCoords[ti2].second; }
                triangles.push_back({static_cast<uint16_t>(vi0), static_cast<uint16_t>(vi1), static_cast<uint16_t>(vi2)});
            }
        }

        // Assign tex coords from vt list if face didn't specify
        for (size_t i = 0; i < vertices.size() && i < texCoords.size(); ++i) {
            if (vertices[i].u == 0.0f && vertices[i].v == 0.0f) {
                vertices[i].u = texCoords[i].first;
                vertices[i].v = texCoords[i].second;
            }
        }

        return !vertices.empty();
    }

    // ── Procedural box helper ─────────────────────────────────────────────
    void addBox(float cx, float cy, float cz, float sx, float sy, float sz,
                float r, float g, float b, float u0, float v0, float u1, float v1) {
        uint16_t base = static_cast<uint16_t>(vertices.size());
        float hx=sx*.5f, hy=sy*.5f, hz=sz*.5f;
        float corners[8][3] = {
            {cx-hx,cy-hy,cz-hz},{cx+hx,cy-hy,cz-hz},{cx+hx,cy+hy,cz-hz},{cx-hx,cy+hy,cz-hz},
            {cx-hx,cy-hy,cz+hz},{cx+hx,cy-hy,cz+hz},{cx+hx,cy+hy,cz+hz},{cx-hx,cy+hy,cz+hz}
        };
        for (int i = 0; i < 8; ++i) {
            float u = (i%2==0)?u0:u1, v = (i/2%2==0)?v0:v1;
            vertices.push_back({corners[i][0],corners[i][1],corners[i][2], r,g,b,1.0f, u,v});
        }
        int faces[6][4] = {{0,1,2,3},{5,4,7,6},{3,2,6,7},{4,5,1,0},{1,5,6,2},{4,0,3,7}};
        for (int f=0;f<6;++f) {
            triangles.push_back({static_cast<uint16_t>(base+faces[f][0]),static_cast<uint16_t>(base+faces[f][1]),static_cast<uint16_t>(base+faces[f][2])});
            triangles.push_back({static_cast<uint16_t>(base+faces[f][0]),static_cast<uint16_t>(base+faces[f][2]),static_cast<uint16_t>(base+faces[f][3])});
        }
    }

    // ── Conker character mesh (ROM OBJ preferred, procedural fallback) ────
    static Model3D createConkerMesh(SDL_Renderer* renderer = nullptr) {
        Model3D m;
        m.name = "Conker (Player Mesh)";
        const char* paths[] = {
            "exported_assets/models/conker_character.obj",
            "../exported_assets/models/conker_character.obj",
            "../../exported_assets/models/conker_character.obj",
            nullptr
        };
        for (int i = 0; paths[i]; ++i) {
            if (m.loadFromOBJ(paths[i], renderer)) {
                std::cout << "[3D] Loaded real Conker ROM mesh: " << m.vertices.size()
                          << " verts, " << m.triangles.size() << " tris" << std::endl;
                return m;
            }
        }
        // Procedural fallback
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

    // ── Level geometry (ROM OBJ preferred, procedural fallback) ──────────
    static Model3D createLevelGeometry(SDL_Renderer* renderer = nullptr) {
        Model3D m;
        m.name = "Level Environment";
        const char* paths[] = {
            "exported_assets/models/assets09_model_000.obj",
            "exported_assets/models/assets0A_model_000.obj",
            "../exported_assets/models/assets09_model_000.obj",
            nullptr
        };
        for (int i = 0; paths[i]; ++i) {
            if (m.loadFromOBJ(paths[i], renderer)) {
                std::cout << "[3D] Loaded real ROM level mesh: " << m.vertices.size()
                          << " verts, " << m.triangles.size() << " tris" << std::endl;
                return m;
            }
        }
        // Procedural fallback
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
};

// ── F3DEX2 Display List processor (unchanged) ─────────────────────────────
class DisplayListProcessor {
public:
    static DisplayListProcessor& getInstance() { static DisplayListProcessor i; return i; }
    void executeDL(uint32_t dlVaddr, Model3D& outMesh) {
        uint8_t* rdram = Memory::getInstance().getRDRAM();
        uint32_t paddr = Memory::getInstance().toPhysical(dlVaddr);
        if (paddr + sizeof(Gfx) > Memory::getInstance().getRDRAMSize()) return;
        std::vector<Vertex3D> vtxBuf(32);
        size_t cmd = 0;
        while (cmd < 2048) {
            uint32_t w0 = Memory::getInstance().read<uint32_t>(dlVaddr + cmd*8);
            uint32_t w1 = Memory::getInstance().read<uint32_t>(dlVaddr + cmd*8 + 4);
            ++cmd;
            uint8_t op = static_cast<uint8_t>(w0 >> 24);
            if (op == GBICommand::G_ENDDL) break;
            else if (op == GBICommand::G_VTX) {
                uint32_t n = (w0>>12)&0xFF;
                for (uint32_t i = 0; i < n && i < 32; ++i) {
                    int16_t vx = Memory::getInstance().read<int16_t>(w1 + i*16);
                    int16_t vy = Memory::getInstance().read<int16_t>(w1 + i*16+2);
                    int16_t vz = Memory::getInstance().read<int16_t>(w1 + i*16+4);
                    vtxBuf[i] = {vx/100.f, vy/100.f, vz/100.f, 1,1,1,1, 0,0};
                }
            }
            else if (op == GBICommand::G_TRI1) {
                uint8_t v0=((w0>>16)&0xFF)/2, v1=((w0>>8)&0xFF)/2, v2=(w0&0xFF)/2;
                uint16_t base = static_cast<uint16_t>(outMesh.vertices.size());
                outMesh.vertices.push_back(vtxBuf[v0]);
                outMesh.vertices.push_back(vtxBuf[v1]);
                outMesh.vertices.push_back(vtxBuf[v2]);
                outMesh.triangles.push_back({base, static_cast<uint16_t>(base+1), static_cast<uint16_t>(base+2)});
            }
        }
    }
};

} // namespace N64
