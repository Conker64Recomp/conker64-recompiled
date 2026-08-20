#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <cstring>
#include <zlib.h>
#include "memory.hpp"

namespace N64 {

// Clave XOR con la que Rareware protegió el segmento ejecutable de Conker
constexpr uint32_t RARE_XOR_KEY = 0x8039CCCA;

struct CodeSubSegment {
    uint32_t startOffset;
    uint32_t endOffset;
    uint32_t length;
    bool compressed;
};

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

    // Descomprime un subsegmento ejecutable de Rareware (4-byte length + raw deflate)
    static std::vector<uint8_t> decompressSubsegment(const uint8_t* data, size_t len) {
        if (len <= 4) return {};

        // 4 bytes de tamaño uncompressed en Big-Endian
        uint32_t uncompSize = (static_cast<uint32_t>(data[0]) << 24) |
                              (static_cast<uint32_t>(data[1]) << 16) |
                              (static_cast<uint32_t>(data[2]) << 8)  |
                               static_cast<uint32_t>(data[3]);

        const uint8_t* payload = data + 4;
        uint32_t payloadLen = static_cast<uint32_t>(len - 4);

        if (uncompSize == 0 || uncompSize > 2 * 1024 * 1024) {
            uncompSize = payloadLen * 8;
        }

        std::vector<uint8_t> out(uncompSize + 1024);

        z_stream zs{};
        zs.next_in   = const_cast<Bytef*>(payload);
        zs.avail_in  = payloadLen;
        zs.next_out  = out.data();
        zs.avail_out = static_cast<uInt>(out.size());

        if (inflateInit2(&zs, -15) != Z_OK) return {};
        int ret = inflate(&zs, Z_FINISH);
        uLong actualSize = zs.total_out;
        inflateEnd(&zs);

        if (ret != Z_STREAM_END && ret != Z_BUF_ERROR) return {};
        out.resize(actualSize);
        return out;
    }

    // Carga y transfiere el código principal del juego (code.bin, 1.98 MB, 508 subsegmentos) a la memoria virtual RDRAM
    static bool loadMainGameCode(const uint8_t* romData, size_t romSize) {
        if (romSize < 0x42450 + 0x15C638) {
            std::cerr << "[RZIP] ROM is too small for game.us.rzip segment!" << std::endl;
            return false;
        }

        const uint8_t* gameRzip = &romData[0x42450];
        size_t gameRzipSize = 0x15C638;

        std::vector<uint32_t> offsets = getDecryptedOffsets(gameRzip, gameRzipSize);
        std::cout << "[RZIP] Decrypted " << offsets.size() << " subsegment offsets in game.us.rzip." << std::endl;

        uint32_t targetVram = 0x80025C2C;
        uint32_t targetPaddr = targetVram & 0x1FFFFFFF;
        uint8_t* rdram = Memory::getInstance().getRDRAM();

        size_t decompressedTotal = 0;
        size_t successfulSegments = 0;

        // Descomprimir y montar secuencialmente los 508 subsegmentos ejecutables en RDRAM
        for (size_t i = 0; i + 1 < offsets.size(); ++i) {
            uint32_t start = offsets[i];
            uint32_t end   = offsets[i + 1];
            if (end <= start || start >= gameRzipSize || end > gameRzipSize) continue;

            size_t segLen = end - start;
            auto decData = decompressSubsegment(gameRzip + start, segLen);

            if (!decData.empty()) {
                uint32_t writeAddr = targetPaddr + static_cast<uint32_t>(decompressedTotal);
                if (writeAddr + decData.size() <= Memory::getInstance().getRDRAMSize()) {
                    std::memcpy(&rdram[writeAddr], decData.data(), decData.size());
                    decompressedTotal += decData.size();
                    successfulSegments++;
                }
            } else {
                // Si es subsegmento sin comprimir, copiar directo
                uint32_t writeAddr = targetPaddr + static_cast<uint32_t>(decompressedTotal);
                if (writeAddr + segLen <= Memory::getInstance().getRDRAMSize()) {
                    std::memcpy(&rdram[writeAddr], gameRzip + start, segLen);
                    decompressedTotal += segLen;
                    successfulSegments++;
                }
            }
        }

        std::cout << "[RZIP] Decompressed & staged " << successfulSegments 
                  << " executable code subsegments (" << decompressedTotal / 1024 
                  << " KB) into RDRAM [0x" << std::hex << targetVram << " - 0x" 
                  << (targetVram + decompressedTotal) << "]" << std::dec << std::endl;

        return true;
    }
};

} // namespace N64
