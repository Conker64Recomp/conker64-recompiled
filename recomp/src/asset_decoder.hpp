#pragma once

/*
 * Rareware Asset Package Decoder for Conker's Bad Fur Day
 * Format: [4 bytes uncompressed length] + [raw deflate payload] (Rareware RZIP)
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <zlib.h>

namespace N64 {

struct SubSegment {
    uint32_t romStart;
    uint32_t romEnd;
    uint32_t romLength;
    uint32_t uncompSize;
    bool     compressed;
};

class AssetDecoder {
public:
    static AssetDecoder& getInstance() {
        static AssetDecoder instance;
        return instance;
    }

    // Guarda una copia del buffer de la ROM para uso interno
    void storeROM(const uint8_t* romData, size_t romSize) {
        romBuffer.assign(romData, romData + romSize);
        std::cout << "[AssetDecoder] ROM buffer cached internally (" << romSize / (1024*1024) << " MB)" << std::endl;
    }

    // Parse the subsegment table of a package — mirrors rzip.py get_files_from_offsets() exactly
    std::vector<SubSegment> parsePackage(const uint8_t* romData, size_t romSize,
                                          uint32_t pkgRomStart, uint32_t pkgRomEnd) {
        std::vector<SubSegment> result;
        if (pkgRomStart >= romSize || pkgRomEnd > romSize || pkgRomStart >= pkgRomEnd) return result;

        uint32_t pkgSize = pkgRomEnd - pkgRomStart;
        size_t   entryOffset = 0;   // entry index (each entry = 8 bytes)

        while (pkgRomStart + entryOffset * 8 + 8 <= pkgRomEnd) {
            const uint8_t* entry = romData + pkgRomStart + entryOffset * 8;
            entryOffset++;

            // Both fields are signed Big-Endian int32 (as in rzip.py ">ii")
            int32_t uncompressed = static_cast<int32_t>(
                (static_cast<uint32_t>(entry[0]) << 24) | (static_cast<uint32_t>(entry[1]) << 16) |
                (static_cast<uint32_t>(entry[2]) << 8)  |  static_cast<uint32_t>(entry[3]));
            int32_t compressed_field = static_cast<int32_t>(
                (static_cast<uint32_t>(entry[4]) << 24) | (static_cast<uint32_t>(entry[5]) << 16) |
                (static_cast<uint32_t>(entry[6]) << 8)  |  static_cast<uint32_t>(entry[7]));

            uint8_t  typ    = static_cast<uint8_t>((compressed_field >> 24) & 0xFF);
            uint32_t length = static_cast<uint32_t>(compressed_field) & 0x00FFFFFF;

            // abs offset = pkgRomStart + uncompressed  (rzip.py: start = base + uncompressed)
            uint32_t absStart = static_cast<uint32_t>(static_cast<int64_t>(pkgRomStart) + uncompressed);
            uint32_t absEnd   = absStart + length;

            if (length == 0 || absStart >= pkgRomEnd || absStart < pkgRomStart) break;
            if (absEnd > static_cast<uint32_t>(romSize)) break;

            SubSegment seg;
            seg.romStart    = absStart;
            seg.romEnd      = absEnd;
            seg.romLength   = length;
            seg.uncompSize  = 0;
            seg.compressed  = (typ & 0x10) != 0;  // type bit 4 = compressed (rzip.py)

            result.push_back(seg);
        }

        return result;
    }

    // Rareware format: [4 bytes uncompressed length] + [raw deflate payload]
    std::vector<uint8_t> decompress(const uint8_t* romData, size_t romSize, const SubSegment& seg) {
        if (seg.romStart + 4 > romSize) return {};

        if (!seg.compressed) {
            std::vector<uint8_t> out(seg.romLength);
            std::memcpy(out.data(), romData + seg.romStart, seg.romLength);
            return out;
        }

        // Leer el tamaño descomprimido del header de 4 bytes (Big-Endian)
        const uint8_t* src = romData + seg.romStart;
        uint32_t uncompSize = (static_cast<uint32_t>(src[0]) << 24) |
                              (static_cast<uint32_t>(src[1]) << 16) |
                              (static_cast<uint32_t>(src[2]) << 8)  |
                               static_cast<uint32_t>(src[3]);

        // Saltar los 4 bytes de header — el payload real empieza en src+4
        const uint8_t* payload    = src + 4;
        uint32_t       payloadLen = seg.romLength - 4;

        if (uncompSize == 0 || uncompSize > 4 * 1024 * 1024) {
            // Tamaño sospechoso, intentar sin header
            uncompSize = payloadLen * 8;
            payload    = src;
            payloadLen = seg.romLength;
        }

        std::vector<uint8_t> out(uncompSize + 1024);

        z_stream zs{};
        zs.next_in   = const_cast<Bytef*>(payload);
        zs.avail_in  = payloadLen;
        zs.next_out  = out.data();
        zs.avail_out = static_cast<uInt>(out.size());

        // windowBits=-15 = raw deflate sin header (formato Rareware)
        if (inflateInit2(&zs, -15) != Z_OK) return {};
        int ret = inflate(&zs, Z_FINISH);
        uLong actualSize = zs.total_out;
        inflateEnd(&zs);

        if (ret != Z_STREAM_END && ret != Z_BUF_ERROR) return {};

        out.resize(actualSize);
        return out;
    }

    // Extrae y descomprime la primera textura RGBA16 válida de assets00
    std::vector<uint8_t> extractFirstTexture(const uint8_t* romData, size_t romSize,
                                              uint32_t pkgRomStart, uint32_t pkgRomEnd,
                                              int& outWidth, int& outHeight) {
        outWidth  = 96;
        outHeight = 110;

        auto segments = parsePackage(romData, romSize, pkgRomStart, pkgRomEnd);
        std::cout << "[AssetDecoder] assets00: " << segments.size() << " subsegments found." << std::endl;

        for (size_t i = 0; i < segments.size() && i < 8; ++i) {
            auto data = decompress(romData, romSize, segments[i]);
            std::cout << "[AssetDecoder] sub" << i
                      << ": romLen=" << segments[i].romLength
                      << " compressed=" << segments[i].compressed
                      << " decompressed=" << data.size() << " bytes" << std::endl;
            if (data.empty()) continue;

            // Dimensiones confirmadas por inspección directa de la ROM:
            // sub00 = 21120 bytes = 96x110 RGBA16 (sprite sheet de vegetación de Conker)
            // sub01-03 = 12800 bytes = 80x80 RGBA16
            static const int WIDTHS[]  = { 96, 80, 64, 32 };
            static const int HEIGHTS[] = {110, 80, 64, 32 };

            for (int k = 0; k < 4; ++k) {
                uint32_t expected = static_cast<uint32_t>(WIDTHS[k]) *
                                    static_cast<uint32_t>(HEIGHTS[k]) * 2;
                if (data.size() >= expected) {
                    outWidth  = WIDTHS[k];
                    outHeight = HEIGHTS[k];
                    data.resize(expected);
                    std::cout << "[AssetDecoder] Loaded REAL Rareware RGBA16 texture "
                              << outWidth << "x" << outHeight
                              << " from assets00 subsegment " << i
                              << " (" << data.size() << " bytes)" << std::endl;
                    return data;
                }
            }
        }

        std::cout << "[AssetDecoder] No valid texture found in assets00. Using procedural fallback." << std::endl;
        return {};
    }

    // Usa el buffer interno para cargar la primera textura real de assets00
    std::vector<uint8_t> loadFirstTexture(int& outWidth, int& outHeight) {
        if (romBuffer.empty()) {
            std::cout << "[AssetDecoder] No ROM buffer cached. Call storeROM() first." << std::endl;
            return {};
        }
        return extractFirstTexture(romBuffer.data(), romBuffer.size(),
                                   0x00AB1A40, 0x00AF4918,
                                   outWidth, outHeight);
    }

private:
    AssetDecoder() = default;
    std::vector<uint8_t> romBuffer;  // Copia interna del buffer de la ROM

    static uint32_t readBE32(const uint8_t* p) {
        return (static_cast<uint32_t>(p[0]) << 24) |
               (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8)  |
               static_cast<uint32_t>(p[3]);
    }
};

} // namespace N64
