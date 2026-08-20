#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <climits>
#include <iostream>

namespace N64 {

// 8MB Expansion Pak RDRAM
constexpr size_t RDRAM_SIZE = 8 * 1024 * 1024;

class Memory {
public:
    static Memory& getInstance() {
        static Memory instance;
        return instance;
    }

    bool init(const std::string& romPath = "") {
        rdram.resize(RDRAM_SIZE, 0);
        std::cout << "[Memory] Allocated " << (RDRAM_SIZE / (1024 * 1024)) << " MB RDRAM." << std::endl;
        return true;
    }

    uint8_t* getRDRAM() { return rdram.data(); }
    size_t getRDRAMSize() const { return rdram.size(); }

    template <typename T>
    T read(uint32_t vaddr) {
        uint32_t paddr = vaddr & 0x1FFFFFFF;
        if (paddr + sizeof(T) <= RDRAM_SIZE) {
            T val;
            std::memcpy(&val, &rdram[paddr], sizeof(T));
            return swapEndian(val);
        }
        return 0;
    }

    template <typename T>
    void write(uint32_t vaddr, T val) {
        uint32_t paddr = vaddr & 0x1FFFFFFF;
        if (paddr + sizeof(T) <= RDRAM_SIZE) {
            T beVal = swapEndian(val);
            std::memcpy(&rdram[paddr], &beVal, sizeof(T));
        }
    }

    uint8_t* getPointer(uint32_t vaddr) {
        uint32_t paddr = vaddr & 0x1FFFFFFF;
        if (paddr < RDRAM_SIZE) {
            return &rdram[paddr];
        }
        return nullptr;
    }

private:
    Memory() = default;
    std::vector<uint8_t> rdram;

    template <typename T>
    static T swapEndian(T u) {
        if constexpr (sizeof(T) == 1) {
            return u;
        } else if constexpr (sizeof(T) == 2) {
            return static_cast<T>((static_cast<uint16_t>(u) >> 8) | (static_cast<uint16_t>(u) << 8));
        } else if constexpr (sizeof(T) == 4) {
            uint32_t val = static_cast<uint32_t>(u);
            return static_cast<T>(((val >> 24) & 0xff) |
                                  ((val << 8) & 0xff0000) |
                                  ((val >> 8) & 0xff00) |
                                  ((val << 24) & 0xff000000));
        } else if constexpr (sizeof(T) == 8) {
            uint64_t val = static_cast<uint64_t>(u);
            val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
            val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
            return static_cast<T>((val << 32) | (val >> 32));
        }
        return u;
    }
};

} // namespace N64
