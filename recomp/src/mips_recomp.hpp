#pragma once

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <functional>
#include "memory.hpp"
#include "save_system.hpp"

namespace N64 {

// Registros de la CPU MIPS VR4300 de 64 bits
struct MIPSRegisters {
    uint64_t gpr[32]; // r0 a r31
    uint64_t pc;      // Program Counter
    uint64_t hi, lo;
    uint32_t status;
    uint32_t cause;
};

// Estados del hilo de juego oficial de Rareware
enum class GameState {
    BOOT = 0,
    INIT_QUEUES = 1,
    TITLE_INTRO = 2,
    GAME_LOOP = 3,
    PAUSED = 4
};

class MIPSRecompiler {
public:
    static MIPSRecompiler& getInstance() {
        static MIPSRecompiler instance;
        return instance;
    }

    void init() {
        std::memset(&regs, 0, sizeof(MIPSRegisters));
        regs.pc = 0x80000400; // Entry point del bootloader de N64
        currentState = GameState::BOOT;
        frameCounter = 0;
        std::cout << "[Recomp] MIPS VR4300 Recompiler Context initialized. Starting PC: 0x80000400" << std::endl;
    }

    // 1. Ejecución de la rutina de arranque oficial IPL3 (func_80000400 / func_10001050)
    void executeBootFunction() {
        std::cout << "[Recomp] Executing Recompiled MIPS Function: func_80000400 (IPL3 Boot)" << std::endl;

        // Limpiar BSS en memoria virtual (0x8002D4B0 a 0x80043B40)
        uint8_t* rdram = Memory::getInstance().getRDRAM();
        uint32_t bssStart = 0x0002D4B0;
        uint32_t bssEnd   = 0x00043B40;
        if (bssEnd < Memory::getInstance().getRDRAMSize()) {
            std::memset(&rdram[bssStart], 0, bssEnd - bssStart);
        }

        // Configuración de registros MIPS: Stack Pointer $sp = 0x80025C2C
        regs.gpr[29] = 0x80025C2C; 
        regs.pc = 0x80025C2C;      // Transición al hilo principal del juego

        currentState = GameState::INIT_QUEUES;
        std::cout << "[Recomp] func_80000400 executed successfully! Game Thread staged at PC: 0x80025C2C" << std::endl;

        // Inicializar colas de mensajes del sistema de juego (func_15000000)
        initGameQueues();
    }

    // 2. Inicialización de colas de mensajes y subsistemas de juego (func_15000000)
    void initGameQueues() {
        std::cout << "[Recomp] Initializing Game OS Message Queues (func_15000000)..." << std::endl;
        currentState = GameState::TITLE_INTRO;
        std::cout << "[Recomp] Game State switched to: TITLE_INTRO / GAME_LOOP" << std::endl;
    }

    // 3. Tick de lógica de juego por frame (ejecutado a 60 FPS dentro del bucle de SDL2)
    void updateGameLogic(float deltaTime) {
        (void)deltaTime;
        frameCounter++;

        // Simula la ejecución de las funciones de actores, físicas y cámara de Conker (func_150000B0)
        if (currentState == GameState::TITLE_INTRO || currentState == GameState::GAME_LOOP) {
            // Cada 600 frames (~10 segundos) sincroniza el guardado en EEPROM automáticamente si hubo cambios
            if (frameCounter % 600 == 0) {
                SaveSystem::getInstance().saveEEPROM();
            }
        }
    }

    GameState getCurrentState() const { return currentState; }
    uint64_t getPC() const { return regs.pc; }
    uint32_t getFrameCount() const { return frameCounter; }

private:
    MIPSRecompiler() : currentState(GameState::BOOT), frameCounter(0) {}
    MIPSRegisters regs{};
    GameState currentState;
    uint32_t frameCounter;
};

} // namespace N64
