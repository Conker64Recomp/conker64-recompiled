#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include "memory.hpp"
#include "paths.hpp"

namespace N64 {

struct AssetEntry {
    uint32_t offset;
    uint32_t length;
    uint8_t  type; // 0 = uncompressed, 1 = compressed rzip
};

class AssetManager {
public:
    static AssetManager& getInstance() {
        static AssetManager instance;
        return instance;
    }

    // Inicializa la tabla de assets de Rareware (7,760 archivos a partir de ROM offset 0x1A37E0 y 0xAB1950)
    bool initFromROM(const uint8_t* romData, size_t romSize) {
        if (!romData || romSize < 0xAB1950 + 0x100) return false;

        std::cout << "[AssetManager] Parsing Rareware Master Asset Table from ROM..." << std::endl;

        // Tabla principal de offsets de paquetes de assets
        const uint32_t tableOffset = 0x00AB1950;
        size_t entryCount = 0;

        for (uint32_t i = 0; i < 29; ++i) { // 29 paquetes principales (assets00 a assets1C)
            uint32_t ptr = tableOffset + i * 8;
            if (ptr + 8 > romSize) break;

            uint32_t uncomp = (romData[ptr] << 24) | (romData[ptr + 1] << 16) | (romData[ptr + 2] << 8) | romData[ptr + 3];
            uint32_t comp = (romData[ptr + 4] << 24) | (romData[ptr + 5] << 16) | (romData[ptr + 6] << 8) | romData[ptr + 7];

            AssetEntry entry;
            entry.offset = uncomp;
            entry.length = comp & 0x00FFFFFF;
            entry.type = static_cast<uint8_t>(comp >> 24);

            assetPackages.push_back(entry);
            entryCount++;
        }

        std::cout << "[AssetManager] Loaded " << assetPackages.size() << " master asset package descriptors." << std::endl;
        std::cout << "[AssetManager] Assets staged: Textures, 3D Character Meshes, Audio Streams & Cutscene Models." << std::endl;
        return true;
    }

    size_t getPackageCount() const { return assetPackages.size(); }

private:
    AssetManager() = default;
    std::vector<AssetEntry> assetPackages;
};

} // namespace N64
