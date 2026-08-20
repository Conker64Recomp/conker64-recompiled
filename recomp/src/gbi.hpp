#pragma once

#include <cstdint>

namespace N64 {

// Estructura oficial del comando GBI (Graphics Binary Interface) de 64 bits
struct Gfx {
    struct {
        uint8_t  cmd;
        uint8_t  flag;
        uint16_t length;
    } words0;
    uint32_t words1;
};

// Comandos principales del microcódigo F3DEX2 de N64 / Rareware
namespace GBICommand {
    constexpr uint8_t G_NOOP           = 0x00;
    constexpr uint8_t G_VTX            = 0x01;
    constexpr uint8_t G_TRI1           = 0x05;
    constexpr uint8_t G_TRI2           = 0x06;
    constexpr uint8_t G_DL             = 0xDE; // Llamar a otra Display List
    constexpr uint8_t G_ENDDL          = 0xDF; // Terminar Display List actual
    constexpr uint8_t G_SETTIMG        = 0xFD; // Establecer textura
    constexpr uint8_t G_SETENVCOLOR    = 0xFB; // Color de ambiente
    constexpr uint8_t G_SETCOMBINE     = 0xFC; // Mezclador de color (Color Combiner)
    constexpr uint8_t G_FILLRECT       = 0xF6; // Rellenar rectángulo 2D
}

struct Vertex3D {
    float x, y, z;
    float r, g, b, a;
    float u, v;
};

} // namespace N64
