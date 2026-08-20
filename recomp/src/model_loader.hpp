#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <SDL.h>
#include "gbi.hpp"

namespace N64 {

class Model3D {
public:
    std::vector<Vertex3D> vertices;
    std::vector<Triangle3D> triangles;
    std::string name;

    // Genera un modelo 3D detallado de Conker en pose de plataforma (cabeza, torso, orejas, gorra, zapatillas)
    static Model3D createConkerMesh() {
        Model3D m;
        m.name = "Conker (Low-Poly N64 Mesh)";

        auto addBox = [&](float cx, float cy, float cz, float sx, float sy, float sz, float r, float g, float b, float u0, float v0, float u1, float v1) {
            uint16_t base = static_cast<uint16_t>(m.vertices.size());
            float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;

            // 8 vertices del cubo/bloque
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
                m.vertices.push_back({corners[i][0], corners[i][1], corners[i][2], r, g, b, 1.0f, u, v});
            }

            // 12 triangulos por bloque (6 caras)
            int faces[6][4] = {
                {0, 1, 2, 3}, // Front
                {5, 4, 7, 6}, // Back
                {3, 2, 6, 7}, // Top
                {4, 5, 1, 0}, // Bottom
                {1, 5, 6, 2}, // Right
                {4, 0, 3, 7}  // Left
            };

            for (int f = 0; f < 6; ++f) {
                m.triangles.push_back({static_cast<uint16_t>(base + faces[f][0]), static_cast<uint16_t>(base + faces[f][1]), static_cast<uint16_t>(base + faces[f][2])});
                m.triangles.push_back({static_cast<uint16_t>(base + faces[f][0]), static_cast<uint16_t>(base + faces[f][2]), static_cast<uint16_t>(base + faces[f][3])});
            }
        };

        // 1. Cabeza de Conker (Naranja brillante)
        addBox(0.0f, 0.5f, 0.0f,  1.1f, 1.0f, 1.0f,  0.95f, 0.50f, 0.15f,  0.0f, 0.0f, 1.0f, 1.0f);

        // 2. Hocico / Mejillas (Blanco / Crema)
        addBox(0.0f, 0.3f, 0.55f,  0.7f, 0.45f, 0.35f,  0.98f, 0.90f, 0.75f,  0.1f, 0.1f, 0.9f, 0.9f);

        // 3. Nariz (Negra)
        addBox(0.0f, 0.45f, 0.75f,  0.22f, 0.18f, 0.18f,  0.10f, 0.10f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);

        // 4. Orejas (Puntiagudas arriba)
        addBox(-0.55f, 1.15f, 0.0f,  0.30f, 0.50f, 0.25f,  0.90f, 0.40f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);
        addBox( 0.55f, 1.15f, 0.0f,  0.30f, 0.50f, 0.25f,  0.90f, 0.40f, 0.10f,  0.0f, 0.0f, 0.5f, 0.5f);

        // 5. Torso / Sudadera Azul con Capucha
        addBox(0.0f, -0.4f, 0.0f,  0.9f, 0.85f, 0.7f,  0.10f, 0.40f, 0.90f,  0.0f, 0.0f, 1.0f, 1.0f);

        // 6. Cierre / Cremallera Amarilla
        addBox(0.0f, -0.4f, 0.36f,  0.12f, 0.80f, 0.05f,  1.0f, 0.85f, 0.10f,  0.0f, 0.0f, 0.2f, 0.2f);

        // 7. Brazos
        addBox(-0.65f, -0.35f, 0.0f,  0.35f, 0.65f, 0.35f,  0.10f, 0.35f, 0.85f,  0.0f, 0.0f, 0.5f, 0.5f);
        addBox( 0.65f, -0.35f, 0.0f,  0.35f, 0.65f, 0.35f,  0.10f, 0.35f, 0.85f,  0.0f, 0.0f, 0.5f, 0.5f);

        // 8. Piernas y Zapatillas Azules/Blancas
        addBox(-0.30f, -1.05f, 0.0f,  0.32f, 0.50f, 0.35f,  0.90f, 0.45f, 0.15f,  0.0f, 0.0f, 0.5f, 0.5f);
        addBox( 0.30f, -1.05f, 0.0f,  0.32f, 0.50f, 0.35f,  0.90f, 0.45f, 0.15f,  0.0f, 0.0f, 0.5f, 0.5f);
        addBox(-0.30f, -1.35f, 0.15f, 0.38f, 0.25f, 0.65f,  0.15f, 0.45f, 0.95f,  0.0f, 0.0f, 1.0f, 1.0f);
        addBox( 0.30f, -1.35f, 0.15f, 0.38f, 0.25f, 0.65f,  0.15f, 0.45f, 0.95f,  0.0f, 0.0f, 1.0f, 1.0f);

        // 9. Cola Esponjosa (Atrás)
        addBox(0.0f, -0.3f, -0.65f,  0.45f, 0.95f, 0.55f,  0.92f, 0.48f, 0.12f,  0.0f, 0.0f, 1.0f, 1.0f);

        return m;
    }
};

} // namespace N64
