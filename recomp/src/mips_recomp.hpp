#pragma once

#include <cstdint>
#include <iostream>
#include "memory.hpp"

namespace N64 {

// Contexto de registros del procesador MIPS VR4300 de N64
struct MIPSContext {
    uint64_t gpr[32]; // Registros $zero ($0) a $ra ($31)
    uint64_t hi;
    uint64_t lo;
    uint32_t pc;
};

class MIPSRecompiler {
public:
    static MIPSRecompiler& getInstance() {
        static MIPSRecompiler instance;
        return instance;
    }

    void init() {
        std::memset(&ctx, 0, sizeof(MIPSContext));
        ctx.pc = 0x80000400; // Entry point inicial de arranque de Conker
        std::cout << "[Recomp] MIPS VR4300 Recompiler Context initialized. Starting PC: 0x80000400" << std::endl;
    }

    // Recompilación en C++ de func_80000400 (Arranque IPL3 oficial de Conker)
    void executeBootFunction() {
        std::cout << "[Recomp] Executing Recompiled MIPS Function: func_80000400 (IPL3 Boot)" << std::endl;

        // 1. Limpiar registros y configurar Stack Pointer ($sp / $29)
        ctx.gpr[29] = 0x80025C2C; // Stack address inicial

        // 2. Limpiar sección BSS de la memoria RDRAM
        uint32_t bssStart = 0x80020000;
        uint32_t bssLength = 0x5C2C;
        uint8_t* rdram = Memory::getInstance().getRDRAM();
        std::memset(&rdram[bssStart & 0x1FFFFFFF], 0, bssLength);

        // 3. Salto a la función principal de inicialización del motor
        ctx.pc = 0x80025C2C;
        std::cout << "[Recomp] func_80000400 executed successfully! Game Thread staged at PC: 0x80025C2C" << std::endl;
    }

    MIPSContext& getContext() { return ctx; }

private:
    MIPSRecompiler() = default;
    MIPSContext ctx;
};

} // namespace N64
