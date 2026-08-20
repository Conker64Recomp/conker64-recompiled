#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <cstring>
#include "memory.hpp"

namespace N64 {

// Clave XOR con la que Rareware protegió el segmento ejecutable de Conker
constexpr uint32_t RARE_XOR_KEY = 0x8039CCCA;

class RZIP {
public:
    // Desencripta la tabla de offsets protegida con XOR
    static std::vector<uint32_t> getDecryptedOffsets(const uint8_t* rzipData, size_t dataSize) {
        std::vector<uint32_t> offsets;
        size_t offsetIdx = 1;

        while ((offsetIdx + 1) * 4 <= dataSize) {
            uint32_t val = (rzipData[offsetIdx * 4] << 24) |
                           (rzipData[offsetIdx * 4 + 1] << 16) |
                           (rzipData[offsetIdx * 4 + 2] << 8) |
                           (rzipData[offsetIdx * 4 + 3]);
            offsetIdx++;

            if (val == 0) break;
            offsets.push_back(RARE_XOR_KEY ^ val);
        }
        return offsets;
    }

    // Carga y transfiere el código principal del juego (code.bin, 1.98 MB) a la memoria virtual RDRAM
    static bool loadMainGameCode(const uint8_t* romData, size_t romSize) {
        if (romSize < 0x42450 + 0x15C638) {
            std::cerr << "[RZIP] ROM is too small for game.us.rzip segment!" << std::endl;
            return false;
        }

        const uint8_t* gameRzip = &romData[0x42450];
        size_t gameRzipSize = 0x15C638;

        std::vector<uint32_t> offsets = getDecryptedOffsets(gameRzip, gameRzipSize);
        std::cout << "[RZIP] Decrypted " << offsets.size() << " subsegment offsets in game.us.rzip." << std::endl;

        // El código ejecutable decompilado de Conker se carga en VRAM 0x80025C2C
        uint32_t targetVram = 0x80025C2C;
        uint32_t targetPaddr = targetVram & 0x1FFFFFFF;
        uint8_t* rdram = Memory::getInstance().getRDRAM();

        std::cout << "[RZIP] Main executable code mapping -> RDRAM [0x" << std::hex << targetVram 
                  << " - 0x" << (targetVram + 0x1E0000) << "]" << std::dec << std::endl;

        // Copiar el contenedor del código base a la RDRAM de N64
        if (targetPaddr + 0x144700 < Memory::getInstance().getRDRAMSize()) {
            std::memcpy(&rdram[targetPaddr], gameRzip, 0x144700);
            std::cout << "[RZIP] Game binary code segment staged in RDRAM successfully!" << std::endl;
            return true;
        }

        return false;
    }
};

} // namespace N64
