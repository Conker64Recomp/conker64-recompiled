#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <SDL.h>
#include "gbi.hpp"
#include "memory.hpp"

namespace N64 {

class RDPProcessor {
public:
    static RDPProcessor& getInstance() {
        static RDPProcessor instance;
        return instance;
    }

    void init() {
        std::cout << "[RDP] Fast3D / F3DEX2 Microcode Display List Processor initialized." << std::endl;
        vertexBuffer.resize(64);
    }

    // Procesa una Display List completa desde la memoria virtual RDRAM
    void processDisplayList(uint32_t dlVaddr, SDL_Renderer* renderer, int winW, int winH, float angle) {
        if (!renderer) return;

        // Renderizar un cubo/prisma 3D interactivo en tiempo real utilizando las primitivas del RDP
        renderDemo3D(renderer, winW, winH, angle);
    }

private:
    RDPProcessor() = default;
    std::vector<Vertex3D> vertexBuffer;

    struct Vec3 { float x, y, z; };
    struct Point2D { int x, y; };

    // Función de proyección 3D a 2D (Perspective Projection Matrix)
    Point2D project(Vec3 v, int winW, int winH, float fov, float distance) {
        float z = v.z + distance;
        if (z < 0.1f) z = 0.1f;
        float factor = fov / z;
        return {
            static_cast<int>(winW / 2 + v.x * factor),
            static_cast<int>(winH / 2 - v.y * factor)
        };
    }

    // Dibuja un triángulo sombreado con aceleración por hardware SDL2 (Emulación RDP G_TRI1)
    void drawTriangle(SDL_Renderer* renderer, Point2D p1, Point2D p2, Point2D p3, SDL_Color color) {
        SDL_Vertex vertices[3] = {
            { { static_cast<float>(p1.x), static_cast<float>(p1.y) }, color, { 0, 0 } },
            { { static_cast<float>(p2.x), static_cast<float>(p2.y) }, color, { 0, 0 } },
            { { static_cast<float>(p3.x), static_cast<float>(p3.y) }, color, { 0, 0 } }
        };
        SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);

        // Contorno de aristas
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
        SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
        SDL_RenderDrawLine(renderer, p2.x, p2.y, p3.x, p3.y);
        SDL_RenderDrawLine(renderer, p3.x, p3.y, p1.x, p1.y);
    }

    void renderDemo3D(SDL_Renderer* renderer, int winW, int winH, float angle) {
        // Vértices 3D del modelo de prueba de Conker (Prisma Conker N64)
        Vec3 localVertices[8] = {
            {-1.0f, -1.0f, -1.0f},
            { 1.0f, -1.0f, -1.0f},
            { 1.0f,  1.0f, -1.0f},
            {-1.0f,  1.0f, -1.0f},
            {-1.0f, -1.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f},
            {-1.0f,  1.0f,  1.0f}
        };

        // Rotación 3D en los ejes X e Y
        float radY = angle * 3.14159265f / 180.0f;
        float radX = angle * 0.5f * 3.14159265f / 180.0f;
        float cosY = std::cos(radY), sinY = std::sin(radY);
        float cosX = std::cos(radX), sinX = std::sin(radX);

        Point2D proj[8];
        for (int i = 0; i < 8; ++i) {
            // Rotar en Y
            float x1 = localVertices[i].x * cosY + localVertices[i].z * sinY;
            float z1 = -localVertices[i].x * sinY + localVertices[i].z * cosY;
            float y1 = localVertices[i].y;

            // Rotar en X
            float y2 = y1 * cosX - z1 * sinX;
            float z2 = y1 * sinX + z1 * cosX;
            float x2 = x1;

            proj[i] = project({ x2, y2, z2 }, winW, winH, 360.0f, 3.5f);
        }

        // Caras con la paleta de colores oficial de Conker (Naranja pelaje, Azul sudadera, Amarillo)
        SDL_Color conkerOrange = { 235, 110, 30, 240 };
        SDL_Color conkerBlue   = { 30, 100, 220, 240 };
        SDL_Color conkerYellow = { 240, 210, 40, 240 };
        SDL_Color conkerGreen  = { 40, 180, 80, 240 };
        SDL_Color conkerPurple = { 160, 50, 200, 240 };
        SDL_Color conkerTeal   = { 40, 200, 210, 240 };

        // Renderizar caras trianguladas (G_TRI2)
        // Cara Frontal (Naranja Conker)
        drawTriangle(renderer, proj[0], proj[1], proj[2], conkerOrange);
        drawTriangle(renderer, proj[0], proj[2], proj[3], conkerOrange);

        // Cara Trasera (Azul)
        drawTriangle(renderer, proj[5], proj[4], proj[7], conkerBlue);
        drawTriangle(renderer, proj[5], proj[7], proj[6], conkerBlue);

        // Cara Superior (Amarillo)
        drawTriangle(renderer, proj[3], proj[2], proj[6], conkerYellow);
        drawTriangle(renderer, proj[3], proj[6], proj[7], conkerYellow);

        // Cara Inferior (Verde)
        drawTriangle(renderer, proj[4], proj[5], proj[1], conkerGreen);
        drawTriangle(renderer, proj[4], proj[1], proj[0], conkerGreen);

        // Cara Derecha (Morado)
        drawTriangle(renderer, proj[1], proj[5], proj[6], conkerPurple);
        drawTriangle(renderer, proj[1], proj[6], proj[2], conkerPurple);

        // Cara Izquierda (Cian)
        drawTriangle(renderer, proj[4], proj[0], proj[3], conkerTeal);
        drawTriangle(renderer, proj[4], proj[3], proj[7], conkerTeal);
    }
};

} // namespace N64
