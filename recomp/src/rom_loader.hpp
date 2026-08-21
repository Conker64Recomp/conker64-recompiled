#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "memory.hpp"
#include "rzip.hpp"
#include "asset_manager.hpp"
#include "asset_decoder.hpp"

namespace N64 {

// Los volcados de cartucho circulan en tres ordenaciones de bytes distintas y
// la extension del archivo NO es fiable (el .n64 del repositorio es en realidad
// un volcado byteswapped). Hay que detectarlo por la firma.
enum class RomByteOrder {
    Z64,      // big-endian nativo:      80 37 12 40
    V64,      // byteswapped 16-bit:     37 80 40 12
    N64,      // little-endian 32-bit:   40 12 37 80
    Unknown
};

struct ROMHeaderInfo {
    uint32_t clockRate = 0;
    uint32_t bootAddress = 0;
    uint32_t crc1 = 0;
    uint32_t crc2 = 0;
    std::string title;
    char countryCode = '?';
    uint8_t version = 0;
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
        if (size < 0x1000) {
            std::cerr << "[ROM Loader] File too small to be an N64 ROM." << std::endl;
            return false;
        }
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(static_cast<size_t>(size));
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            std::cerr << "[ROM Loader] Error reading ROM file." << std::endl;
            return false;
        }

        // Normalizar a z64 antes de tocar nada mas. Sin esto, un volcado v64/n64
        // se parseaba como si fuera big-endian y producia basura en silencio.
        RomByteOrder order = detectByteOrder(buffer.data());
        switch (order) {
            case RomByteOrder::Z64:
                std::cout << "[ROM Loader] Byte order: z64 (big-endian, native)." << std::endl;
                break;
            case RomByteOrder::V64:
                std::cout << "[ROM Loader] Byte order: v64 (byteswapped) -> converting to z64." << std::endl;
                convertV64ToZ64(buffer);
                break;
            case RomByteOrder::N64:
                std::cout << "[ROM Loader] Byte order: n64 (little-endian) -> converting to z64." << std::endl;
                convertN64ToZ64(buffer);
                break;
            case RomByteOrder::Unknown:
                std::cerr << "[ROM Loader] Unrecognized ROM signature: not a valid N64 dump." << std::endl;
                return false;
        }

        std::cout << "[ROM Loader] ROM loaded (" << (size / (1024 * 1024)) << " MB)" << std::endl;

        ROMHeaderInfo header = parseHeader(buffer.data());
        std::cout << "[ROM Header] Title:      " << header.title << std::endl;
        std::cout << "[ROM Header] Boot PC:    0x" << std::hex << std::uppercase
                  << header.bootAddress << std::dec << std::nouppercase << std::endl;
        std::cout << "[ROM Header] CRC1/CRC2:  0x" << std::hex << std::uppercase
                  << header.crc1 << " / 0x" << header.crc2
                  << std::dec << std::nouppercase << std::endl;
        std::cout << "[ROM Header] Region:     " << header.countryCode
                  << " (version " << static_cast<int>(header.version) << ")" << std::endl;

        // Cargar IPL3 + init.us.bin (0x1000 a 0x25C2C) en RDRAM (0x80000400)
        uint8_t* rdram = Memory::getInstance().getRDRAM();
        if (size >= 0x25C2C) {
            constexpr uint32_t destPaddr    = 0x00000400;
            constexpr uint32_t srcRomOffset = 0x1000;
            constexpr uint32_t copySize     = 0x25C2C - 0x1000;

            std::memcpy(&rdram[destPaddr], &buffer[srcRomOffset], copySize);
            std::cout << "[Memory] Boot code (IPL3/Init) staged in RDRAM [0x80000400 - 0x80025C2C] ("
                      << (copySize / 1024) << " KB)" << std::endl;
        }

        RZIP::loadMainGameCode(buffer.data(), buffer.size());
        AssetManager::getInstance().initFromROM(buffer.data(), buffer.size());
        AssetDecoder::getInstance().storeROM(buffer.data(), buffer.size());

        return true;
    }

private:
    static RomByteOrder detectByteOrder(const uint8_t* d) {
        if (d[0] == 0x80 && d[1] == 0x37 && d[2] == 0x12 && d[3] == 0x40) return RomByteOrder::Z64;
        if (d[0] == 0x37 && d[1] == 0x80 && d[2] == 0x40 && d[3] == 0x12) return RomByteOrder::V64;
        if (d[0] == 0x40 && d[1] == 0x12 && d[2] == 0x37 && d[3] == 0x80) return RomByteOrder::N64;
        return RomByteOrder::Unknown;
    }

    // v64: intercambia los dos bytes de cada palabra de 16 bits.
    static void convertV64ToZ64(std::vector<uint8_t>& buf) {
        size_t n = buf.size() & ~static_cast<size_t>(1);
        for (size_t i = 0; i < n; i += 2) {
            std::swap(buf[i], buf[i + 1]);
        }
    }

    // n64: invierte por completo cada palabra de 32 bits.
    static void convertN64ToZ64(std::vector<uint8_t>& buf) {
        size_t n = buf.size() & ~static_cast<size_t>(3);
        for (size_t i = 0; i < n; i += 4) {
            std::swap(buf[i],     buf[i + 3]);
            std::swap(buf[i + 1], buf[i + 2]);
        }
    }

    static uint32_t readBE32(const uint8_t* p) {
        return (static_cast<uint32_t>(p[0]) << 24) |
               (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8)  |
                static_cast<uint32_t>(p[3]);
    }

    // Lectura explicita big-endian campo a campo. El memcpy anterior a un struct
    // no hacia byte swap ni respetaba el layout, e imprimia 0x100080 donde la
    // ROM dice 0x80001000.
    static ROMHeaderInfo parseHeader(const uint8_t* d) {
        ROMHeaderInfo h;
        h.clockRate   = readBE32(d + 0x04);
        h.bootAddress = readBE32(d + 0x08);
        h.crc1        = readBE32(d + 0x10);
        h.crc2        = readBE32(d + 0x14);

        char title[21] = { 0 };
        std::memcpy(title, d + 0x20, 20);
        h.title = title;
        while (!h.title.empty() && (h.title.back() == ' ' || h.title.back() == '\0')) {
            h.title.pop_back();
        }

        h.countryCode = static_cast<char>(d[0x3E]);
        h.version     = d[0x3F];
        return h;
    }
};

} // namespace N64
