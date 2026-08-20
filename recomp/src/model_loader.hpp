#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>
#include <SDL.h>
#include "gbi.hpp"
#include "memory.hpp"

namespace N64 {

class Model3D {
public:
    std::vector<Vertex3D> vertices;
    std::vector<Triangle3D> triangles;
    std::string name;

    // Helper para añadir bloques 3D poligonales
    void addBox(float cx, float cy, float cz, float sx, float sy, float sz, 
                float r, float g, float b, float u0, float v0, float u1, float v1) {
        uint16_t base = static_cast<uint16_t>(vertices.size());
        float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;

        float corners[8][3] = {
            {cx - hx, cy - hy, cz - hz}, // 0
            {cx + hx, cy - hy, cz - hz}, // 1
            {cx + hx, cy + hy, cz - hz}, // 2
            {cx - hx, cy + hy, cz - hz}, // 3
            {cx - hx, cy - hy, cz + hz}, // 4
            {cx + hx, cy - hy, cz + hz}, // 5
            {cx + hx, cy + hy, cz + hz}, // 6
            {cx - hx, cy + hy, cz + hz}  // 7
        };

        for (int i = 0; i < 8; ++i) {
            float u = (i % 2 == 0) ? u0 : u1;
            float v = (i / 2 % 2 == 0) ? v0 : v1;
            vertices.push_back({corners[i][0], corners[i][1], corners[i][2], r, g, b, 1.0f, u, v});
        }

        int faces[6][4] = {
            {0, 1, 2, 3}, // Front
            {5, 4, 7, 6}, // Back
            {3, 2, 6, 7}, // Top
            {4, 5, 1, 0}, // Bottom
            {1, 5, 6, 2}, // Right
            {4, 0, 3, 7}  // Left
        };

        for (int f = 0; f < 6; ++f) {
            triangles.push_back({static_cast<uint16_t>(base + faces[f][0]), static_cast<uint16_t>(base + faces[f][1]), static_cast<uint16_t>(base + faces[f][2])});
            triangles.push_back({static_cast<uint16_t>(base + faces[f][0]), static_cast<uint16_t>(base + faces[f][2]), static_cast<uint16_t>(base + faces[f][3])});
        }
    }

    // 1. Malla 3D del personaje principal (Conker)
    static Model3D createConkerMesh() {
        Model3D m;
        m.name = "Conker (Player Mesh)";

        // Cabeza
        m.addBox(0.0f, 0.5f, 0.0f,  1.1f, 1.0f, 1.0f,  0.95f, 0.50f, 0.15f,  0.0f, 0.0f, 1.0f, 1.0f);
        // Hocico / Mejillas
        m.addBox(0.0f, 0.3f, 0.55f,  0.7f, 0.45f, 0.35f,  0.98f, 0.90f, 0.75f,  0.1f, 0.1f, 0.9f, 0.9f);
        // Nariz
        m.addBox(0.0f, 0.45f, 0.75f,  0.22f, 0.18f, 0.18f,  0.10f, 0.10f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);
        // Orejas
        m.addBox(-0.55f, 1.15f, 0.0f,  0.30f, 0.50f, 0.25f,  0.90f, 0.40f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);
        m.addBox( 0.55f, 1.15f, 0.0f,  0.30f, 0.50f, 0.25f,  0.90f, 0.40f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);
        // Torso / Sudadera Azul
        m.addBox(0.0f, -0.4f, 0.0f,  0.9f, 0.85f, 0.7f,  0.10f, 0.40f, 0.90f,  0.0f, 0.0f, 1.0f, 1.0f);
        // Cierre Amarillo
        m.addBox(0.0f, -0.4f, 0.36f,  0.12f, 0.80f, 0.05f,  1.0f, 0.85f, 0.10f,  0.0f, 0.0f, 0.2f, 0.2f);
        // Brazos
        m.addBox(-0.65f, -0.35f, 0.0f,  0.35f, 0.65f, 0.35f,  0.10f, 0.35f, 0.85f,  0.0f, 0.0f, 0.5f, 0.5f);
        m.addBox( 0.65f, -0.35f, 0.0f,  0.35f, 0.65f, 0.35f,  0.10f, 0.35f, 0.85f,  0.0f, 0.0f, 0.5f, 0.5f);
        // Piernas y Zapatillas
        m.addBox(-0.30f, -1.05f, 0.0f,  0.32f, 0.50f, 0.35f,  0.90f, 0.45f, 0.15f,  0.0f, 0.0f, 0.5f, 0.5f);
        m.addBox( 0.30f, -1.05f, 0.0f,  0.32f, 0.50f, 0.35f,  0.90f, 0.45f, 0.15f,  0.0f, 0.0f, 0.5f, 0.5f);
        m.addBox(-0.30f, -1.35f, 0.15f, 0.38f, 0.25f, 0.65f,  0.15f, 0.45f, 0.95f,  0.0f, 0.0f, 1.0f, 1.0f);
        m.addBox( 0.30f, -1.35f, 0.15f, 0.38f, 0.25f, 0.65f,  0.15f, 0.45f, 0.95f,  0.0f, 0.0f, 1.0f, 1.0f);
        // Cola
        m.addBox(0.0f, -0.3f, -0.65f,  0.45f, 0.95f, 0.55f,  0.92f, 0.48f, 0.12f,  0.0f, 0.0f, 1.0f, 1.0f);

        return m;
    }

    // 2. Malla 3D del escenario interactivo (Plataformas de madera, colinas, y botón Context Sensitive "B")
    static Model3D createLevelGeometry() {
        Model3D m;
        m.name = "Hungover Area (Level Environment & Props)";

        // Plataforma Context Sensitive (Botón "B" de madera roja de Rareware)
        m.addBox(0.0f, -1.25f, 5.0f,  2.8f, 0.20f, 2.8f,  0.85f, 0.25f, 0.15f,  0.0f, 0.0f, 1.0f, 1.0f);
        m.addBox(0.0f, -1.13f, 5.0f,  1.8f, 0.10f, 1.8f,  1.00f, 0.80f, 0.10f,  0.2f, 0.2f, 0.8f, 0.8f);

        // Plataforma elevada de roca/madera (Izquierda)
        m.addBox(-5.5f, -0.5f, 4.0f,  3.5f, 1.5f, 3.5f,  0.55f, 0.35f, 0.20f,  0.0f, 0.0f, 1.0f, 1.0f);
        // Plataforma escalonada (Derecha)
        m.addBox( 5.5f, -0.2f, 4.0f,  3.5f, 2.0f, 3.5f,  0.45f, 0.45f, 0.45f,  0.0f, 0.0f, 1.0f, 1.0f);

        // Barril de cerveza / taberna (Prop clásico)
        m.addBox(-5.5f, 0.75f, 4.0f,  1.2f, 1.5f, 1.2f,  0.60f, 0.30f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);

        // Árboles y Troncos en el entorno
        m.addBox( 6.0f, 1.2f, 8.0f,  0.6f, 3.0f, 0.6f,  0.40f, 0.20f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);
        m.addBox( 6.0f, 3.0f, 8.0f,  2.5f, 1.5f, 2.5f,  0.15f, 0.65f, 0.20f,  0.0f, 0.0f, 1.0f, 1.0f);

        m.addBox(-6.0f, 1.2f, 8.0f,  0.6f, 3.0f, 0.6f,  0.40f, 0.20f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);
        m.addBox(-6.0f, 3.0f, 8.0f,  2.5f, 1.5f, 2.5f,  0.15f, 0.65f, 0.20f,  0.0f, 0.0f, 1.0f, 1.0f);

        return m;
    }
};

// Intérprete de Display Lists de microcódigo F3DEX2
class DisplayListProcessor {
public:
    static DisplayListProcessor& getInstance() {
        static DisplayListProcessor instance;
        return instance;
    }

    void executeDL(uint32_t dlVaddr, Model3D& outMesh) {
        uint8_t* rdram = Memory::getInstance().getRDRAM();
        uint32_t paddr = Memory::getInstance().toPhysical(dlVaddr);

        if (paddr + sizeof(Gfx) > Memory::getInstance().getRDRAMSize()) return;

        std::vector<Vertex3D> vtxBuffer(32);
        size_t cmdCount = 0;

        while (cmdCount < 2048) {
            uint32_t w0 = Memory::getInstance().read<uint32_t>(dlVaddr + cmdCount * 8);
            uint32_t w1 = Memory::getInstance().read<uint32_t>(dlVaddr + cmdCount * 8 + 4);
            cmdCount++;

            uint8_t opcode = static_cast<uint8_t>(w0 >> 24);

            if (opcode == GBICommand::G_ENDDL) {
                break; // Fin de Display List
            }
            else if (opcode == GBICommand::G_VTX) {
                // Cargar buffer de vértices
                uint32_t numVtx = (w0 >> 12) & 0xFF;
                uint32_t vaddrSrc = w1;
                for (uint32_t i = 0; i < numVtx && i < 32; ++i) {
                    int16_t vx = Memory::getInstance().read<int16_t>(vaddrSrc + i * 16);
                    int16_t vy = Memory::getInstance().read<int16_t>(vaddrSrc + i * 16 + 2);
                    int16_t vz = Memory::getInstance().read<int16_t>(vaddrSrc + i * 16 + 4);
                    vtxBuffer[i] = { vx / 100.0f, vy / 100.0f, vz / 100.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f };
                }
            }
            else if (opcode == GBICommand::G_TRI1) {
                uint8_t v0 = static_cast<uint8_t>((w0 >> 16) & 0xFF) / 2;
                uint8_t v1 = static_cast<uint8_t>((w0 >> 8) & 0xFF) / 2;
                uint8_t v2 = static_cast<uint8_t>(w0 & 0xFF) / 2;

                uint16_t base = static_cast<uint16_t>(outMesh.vertices.size());
                outMesh.vertices.push_back(vtxBuffer[v0]);
                outMesh.vertices.push_back(vtxBuffer[v1]);
                outMesh.vertices.push_back(vtxBuffer[v2]);
                outMesh.triangles.push_back({ base, static_cast<uint16_t>(base + 1), static_cast<uint16_t>(base + 2) });
            }
        }
    }
};

} // namespace N64
