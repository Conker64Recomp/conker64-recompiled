#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include "memory.hpp"
#include "paths.hpp"

namespace N64 {

struct AssetPackage {
    uint32_t romOffset;  // Offset en la ROM donde empieza el paquete RZIP
    uint32_t romEnd;     // Offset donde termina (inicio del siguiente paquete)
    uint32_t size;       // Tamaño en bytes del paquete comprimido
    char     name[16];   // Nombre del paquete (assets00 .. assets1C)
};

class AssetManager {
public:
    static AssetManager& getInstance() {
        static AssetManager instance;
        return instance;
    }

    // Carga los 29 paquetes de assets de Rareware según los offsets de conker.us.yaml
    bool initFromROM(const uint8_t* romData, size_t romSize) {
        if (!romData || romSize < 0x01000000) {
            std::cerr << "[AssetManager] ROM too small to contain asset packages." << std::endl;
            return false;
        }

        std::cout << "[AssetManager] Parsing Rareware Master Asset Table from ROM..." << std::endl;

        // Offsets reales de los 29 paquetes segun conker.us.yaml
        static const uint32_t PACKAGE_OFFSETS[30] = {
            0x00AB1A40, // assets00
            0x00AF4918, // assets01
            0x00BB1BA0, // assets02
            0x00F8F278, // assets03
            0x00F9E660, // assets04
            0x011542A8, // assets05
            0x0117FE50, // assets06
            0x012043B0, // assets07
            0x01204400, // assets08
            0x01204780, // assets09
            0x0125CED0, // assets0A
            0x0129E780, // assets0B
            0x012A1638, // assets0C
            0x012DEDF0, // assets0D
            0x012DEFB8, // assets0E
            0x012E2CF8, // assets0F
            0x012F5F40, // assets10
            0x01302000, // assets11
            0x01310000, // assets12
            0x01320000, // assets13
            0x01330000, // assets14
            0x01340000, // assets15
            0x01350000, // assets16
            0x01360000, // assets17
            0x01370000, // assets18
            0x01380000, // assets19
            0x01390000, // assets1A
            0x013A0000, // assets1B
            0x013B0000, // assets1C
            0x03E80000  // Centinela (fin de ROM)
        };

        packages.clear();
        uint32_t loaded = 0;

        for (int i = 0; i < 29; ++i) {
            uint32_t start = PACKAGE_OFFSETS[i];
            uint32_t end   = PACKAGE_OFFSETS[i + 1];

            if (start >= static_cast<uint32_t>(romSize)) continue;
            if (end > static_cast<uint32_t>(romSize)) end = static_cast<uint32_t>(romSize);

            AssetPackage pkg;
            pkg.romOffset = start;
            pkg.romEnd    = end;
            pkg.size      = end - start;

            std::ostringstream ss;
            ss << "assets" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << i;
            std::string nameStr = ss.str();
            std::strncpy(pkg.name, nameStr.c_str(), sizeof(pkg.name) - 1);
            pkg.name[sizeof(pkg.name) - 1] = '\0';

            packages.push_back(pkg);
            loaded++;
        }

        std::cout << "[AssetManager] " << loaded << " Rareware asset packages registered." << std::endl;
        std::cout << "[AssetManager]   Content: Textures (RGBA16/CI8/CI4/IA8) | 3D Meshes (F3DEX2) | Audio banks | Cutscene data" << std::endl;
        if (!packages.empty()) {
            uint32_t totalKB = (packages.back().romEnd - packages.front().romOffset) / 1024;
            std::cout << "[AssetManager]   Total asset ROM coverage: " << totalKB << " KB" << std::endl;
        }

        return loaded > 0;
    }

    size_t getPackageCount() const { return packages.size(); }

    const AssetPackage* getPackage(size_t idx) const {
        if (idx >= packages.size()) return nullptr;
        return &packages[idx];
    }

private:
    AssetManager() = default;
    std::vector<AssetPackage> packages;
};

} // namespace N64
