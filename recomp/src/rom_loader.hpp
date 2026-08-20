#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include "memory.hpp"

namespace N64 {

struct ROMHeader {
    uint8_t  pi_bsb_dom1_lat_reg;
    uint8_t  pi_bsb_dom1_pgs_reg;
    uint8_t  pi_bsb_dom1_pwd_reg;
    uint8_t  pi_bsb_dom1_pgs_reg2;
    uint32_t clock_rate;
    uint32_t program_counter;
    uint32_t release;
    uint32_t crc1;
    uint32_t crc2;
    uint64_t unknown;
    char     game_name[20];
    uint32_t unknown2;
    uint32_t media_format;
    char     cartridge_id[2];
    char     country_code;
    uint8_t  version;
};

class ROMLoader {
public:
    static bool loadROM(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "[ROM Loader] Failed to open ROM: " << filepath << std::endl;
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            std::cerr << "[ROM Loader] Error reading ROM file." << std::endl;
            return false;
        }

        std::cout << "[ROM Loader] ROM loaded successfully (" << (size / (1024 * 1024)) << " MB)" << std::endl;

        // Leer cabecera oficial de Nintendo 64
        if (size >= 0x40) {
            ROMHeader header;
            std::memcpy(&header, buffer.data(), sizeof(ROMHeader));
            
            char title[21] = { 0 };
            std::memcpy(title, header.game_name, 20);
            std::cout << "[ROM Header] Internal Title: " << title << std::endl;
            std::cout << "[ROM Header] Entry Point PC: 0x" << std::hex << header.program_counter << std::dec << std::endl;
            std::cout << "[ROM Header] Country / Region: " << header.country_code << std::endl;
        }

        // Cargar el segmento de arranque IPL3 e init.us.bin (0x1000 a 0x25C2C) en RDRAM (0x80000400)
        uint8_t* rdram = Memory::getInstance().getRDRAM();
        if (size >= 0x25C2C) {
            // El IPL3 de N64 copia desde ROM 0x1000 a RDRAM 0x80000400
            uint32_t destPaddr = 0x00000400;
            uint32_t srcRomOffset = 0x1000;
            uint32_t copySize = 0x25C2C - 0x1000;

            std::memcpy(&rdram[destPaddr], &buffer[srcRomOffset], copySize);
            std::cout << "[Memory] Boot code (IPL3/Init) DMA transferred to RDRAM [0x80000400 - 0x80025C2C] ("
                      << (copySize / 1024) << " KB)" << std::endl;
        }

        return true;
    }
};

} // namespace N64
