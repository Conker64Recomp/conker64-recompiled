#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <climits>
#include <iostream>

namespace N64 {

// 8MB Expansion Pak RDRAM (Conker requiere Expansion Pak obligatorio de 8MB)
constexpr size_t RDRAM_SIZE = 8 * 1024 * 1024;

// Registros de Hardware de Nintendo 64 (Memory-Mapped I/O)
namespace MMIO {
    // MIPS Interface (MI)
    constexpr uint32_t MI_BASE_REG       = 0x04300000;
    constexpr uint32_t MI_INIT_MODE_REG  = 0x04300000;
    constexpr uint32_t MI_VERSION_REG    = 0x04300004;
    constexpr uint32_t MI_INTR_REG       = 0x04300008;
    constexpr uint32_t MI_INTR_MASK_REG  = 0x0430000C;

    // Video Interface (VI)
    constexpr uint32_t VI_BASE_REG       = 0x04400000;
    constexpr uint32_t VI_STATUS_REG     = 0x04400000;
    constexpr uint32_t VI_ORIGIN_REG     = 0x04400004;
    constexpr uint32_t VI_WIDTH_REG      = 0x04400008;
    constexpr uint32_t VI_INTR_REG       = 0x0440000C;
    constexpr uint32_t VI_V_CURRENT_LINE = 0x04400010;

    // Audio Interface (AI)
    constexpr uint32_t AI_BASE_REG       = 0x04500000;
    constexpr uint32_t AI_DRAM_ADDR_REG  = 0x04500000;
    constexpr uint32_t AI_LEN_REG        = 0x04500004;
    constexpr uint32_t AI_CONTROL_REG    = 0x04500008;
    constexpr uint32_t AI_STATUS_REG     = 0x0450000C;

    // Peripheral Interface (PI - Cartridge ROM DMA)
    constexpr uint32_t PI_BASE_REG       = 0x04600000;
    constexpr uint32_t PI_DRAM_ADDR_REG  = 0x04600000;
    constexpr uint32_t PI_CART_ADDR_REG  = 0x04600004;
    constexpr uint32_t PI_RD_LEN_REG     = 0x04600008;
    constexpr uint32_t PI_WR_LEN_REG     = 0x0460000C;
    constexpr uint32_t PI_STATUS_REG     = 0x04600010;

    // Serial Interface (SI - Controllers & EEPROM)
    constexpr uint32_t SI_BASE_REG       = 0x04800000;
    constexpr uint32_t SI_DRAM_ADDR_REG  = 0x04800000;
    constexpr uint32_t SI_PIF_ADDR_RD64B = 0x04800004;
    constexpr uint32_t SI_PIF_ADDR_WR64B = 0x04800010;
    constexpr uint32_t SI_STATUS_REG     = 0x04800018;
}

class Memory {
public:
    static Memory& getInstance() {
        static Memory instance;
        return instance;
    }

    bool init(const std::string& romPath = "") {
        (void)romPath;
        rdram.resize(RDRAM_SIZE, 0);
        mmioRegisters.resize(0x100000, 0); // 1MB para registros de hardware N64
        std::cout << "[Memory] Full N64 MMU Engine initialized (8 MB RDRAM + Hardware MMIO Registers)." << std::endl;
        return true;
    }

    uint8_t* getRDRAM() { return rdram.data(); }
    size_t getRDRAMSize() const { return rdram.size(); }

    // Mapeo de dirección virtual MIPS a física (KSEG0: 0x80000000, KSEG1: 0xA0000000, KUSEG: 0x00000000)
    inline uint32_t toPhysical(uint32_t vaddr) const {
        return vaddr & 0x1FFFFFFF;
    }

    template <typename T>
    T read(uint32_t vaddr) {
        uint32_t paddr = toPhysical(vaddr);

        // Lectura de memoria RDRAM (0x00000000 - 0x007FFFFF)
        if (paddr + sizeof(T) <= RDRAM_SIZE) {
            T val;
            std::memcpy(&val, &rdram[paddr], sizeof(T));
            return swapEndian(val);
        }

        // Lectura de registros de hardware MMIO (0x04000000 - 0x048FFFFF)
        if (paddr >= 0x04000000 && paddr < 0x04900000) {
            uint32_t offset = paddr - 0x04000000;
            if (offset + sizeof(T) <= mmioRegisters.size()) {
                T val;
                std::memcpy(&val, &mmioRegisters[offset], sizeof(T));
                return swapEndian(val);
            }
        }

        return 0;
    }

    template <typename T>
    void write(uint32_t vaddr, T val) {
        uint32_t paddr = toPhysical(vaddr);

        // Escritura en RDRAM
        if (paddr + sizeof(T) <= RDRAM_SIZE) {
            T beVal = swapEndian(val);
            std::memcpy(&rdram[paddr], &beVal, sizeof(T));
            return;
        }

        // Escritura en registros MMIO
        if (paddr >= 0x04000000 && paddr < 0x04900000) {
            uint32_t offset = paddr - 0x04000000;
            if (offset + sizeof(T) <= mmioRegisters.size()) {
                T beVal = swapEndian(val);
                std::memcpy(&mmioRegisters[offset], &beVal, sizeof(T));
            }
        }
    }

    uint8_t* getPointer(uint32_t vaddr) {
        uint32_t paddr = toPhysical(vaddr);
        if (paddr < RDRAM_SIZE) {
            return &rdram[paddr];
        }
        return nullptr;
    }

    // Transferencia DMA de Cartucho a RDRAM (PI DMA)
    void dmaCopyFromROM(const uint8_t* romData, size_t romSize, uint32_t romOffset, uint32_t rdramDestVaddr, size_t length) {
        if (!romData || romOffset + length > romSize) return;

        uint32_t paddr = toPhysical(rdramDestVaddr);
        if (paddr + length <= RDRAM_SIZE) {
            std::memcpy(&rdram[paddr], romData + romOffset, length);
            std::cout << "[Memory-DMA] Copied 0x" << std::hex << length 
                      << " bytes from ROM 0x" << romOffset 
                      << " -> RDRAM 0x" << rdramDestVaddr << std::dec << std::endl;
        }
    }

private:
    Memory() = default;
    std::vector<uint8_t> rdram;
    std::vector<uint8_t> mmioRegisters;

    template <typename T>
    static T swapEndian(T val) {
        if constexpr (sizeof(T) == 2) {
            uint16_t v = static_cast<uint16_t>(val);
            return static_cast<T>((v >> 8) | (v << 8));
        } else if constexpr (sizeof(T) == 4) {
            uint32_t v = static_cast<uint32_t>(val);
            return static_cast<T>(((v >> 24) & 0xFF) | ((v >> 8) & 0xFF00) |
                                  ((v << 8) & 0xFF0000) | ((v << 24) & 0xFF000000));
        } else if constexpr (sizeof(T) == 8) {
            uint64_t v = static_cast<uint64_t>(val);
            return static_cast<T>(
                ((v >> 56) & 0x00000000000000FFULL) |
                ((v >> 40) & 0x000000000000FF00ULL) |
                ((v >> 24) & 0x0000000000FF0000ULL) |
                ((v >> 8)  & 0x00000000FF000000ULL) |
                ((v << 8)  & 0x000000FF00000000ULL) |
                ((v << 24) & 0x0000FF0000000000ULL) |
                ((v << 40) & 0x00FF000000000000ULL) |
                ((v << 56) & 0xFF00000000000000ULL));
        }
        return val;
    }
};

} // namespace N64
