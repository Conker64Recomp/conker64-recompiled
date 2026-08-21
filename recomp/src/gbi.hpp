#pragma once

#include <cstdint>

namespace N64 {

// Estructura oficial del comando GBI (Graphics Binary Interface) de 64 bits de F3DEX2
struct Gfx {
    uint32_t w0;
    uint32_t w1;
};

// Comandos de microcódigo F3DEX2 (Nintendo 64 / Rareware)
namespace GBICommand {
    constexpr uint8_t G_NOOP         = 0x00;
    constexpr uint8_t G_VTX          = 0x01; // Cargar buffer de vertices
    constexpr uint8_t G_MODIFYVTX    = 0x02;
    constexpr uint8_t G_CULLDL       = 0x03;
    constexpr uint8_t G_TRI1         = 0x05; // 1 Triangulo
    constexpr uint8_t G_TRI2         = 0x06; // 2 Triangulos
    constexpr uint8_t G_QUAD         = 0x07;
    constexpr uint8_t G_POPMTX       = 0xD8;
    constexpr uint8_t G_MTX          = 0xDA; // Cargar matriz de transformacion
    constexpr uint8_t G_DL           = 0xDE; // Branch a Display List
    constexpr uint8_t G_ENDDL        = 0xDF; // Fin de Display List
    constexpr uint8_t G_SETTIMG      = 0xFD; // Puntero de Textura
    constexpr uint8_t G_SETCOMBINE   = 0xFC; // Color Combiner
    constexpr uint8_t G_SETENVCOLOR  = 0xFB; // Color de ambiente
    constexpr uint8_t G_SETPRIMCOLOR = 0xFA; // Color primario
    constexpr uint8_t G_SETTILE      = 0xF5; // Descriptor de Tile de Textura
    constexpr uint8_t G_SETTILESIZE  = 0xF2; // Dimensiones de Tile
}

// Estructura oficial de un vertice N64 (16 bytes en memoria RDRAM)
struct Vtx_t {
    int16_t ob[3];  // Posicion x, y, z en espacio de modelo (coordenadas fijas)
    uint16_t flag;
    int16_t tc[2];  // Coordenadas de textura s, t (fijo 10.5)
    uint8_t cn[4];  // Color / Normal r, g, b, a
};

union Vtx {
    Vtx_t v;
    uint32_t raw[4];
};

struct Vertex3D {
    float x, y, z;
    float r, g, b, a;
    float u, v;
};

// Indices de 32 bits: al fusionar la geometria real de la ROM en una sola malla
// se superan facilmente los 65535 vertices, y el uint16_t anterior desbordaba en
// silencio produciendo triangulos que apuntaban a vertices arbitrarios.
struct Triangle3D {
    uint32_t v0, v1, v2;
};

} // namespace N64
