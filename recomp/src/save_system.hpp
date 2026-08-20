#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include "paths.hpp"
#include "memory.hpp"

namespace N64 {

// Conker's Bad Fur Day utiliza un chip EEPROM de 16Kbit (2048 bytes / 256 bloques de 8 bytes)
constexpr size_t EEPROM_SIZE = 2048;
constexpr size_t EEPROM_BLOCK_SIZE = 8;
constexpr size_t EEPROM_NUM_BLOCKS = EEPROM_SIZE / EEPROM_BLOCK_SIZE;

class SaveSystem {
public:
    static SaveSystem& getInstance() {
        static SaveSystem instance;
        return instance;
    }

    void init() {
        eepromData.resize(EEPROM_SIZE, 0xFF); // En cartucho real se inicializa en 0xFF
        loadEEPROM();
    }

    // 1. Cargar archivo de guardado nativo del juego desde AppData al iniciar
    bool loadEEPROM() {
        std::string path = PathManager::getSaveFilePath();
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "[SaveSystem] No existing in-game save found. Ready for fresh game progress." << std::endl;
            return false;
        }

        file.read(reinterpret_cast<char*>(eepromData.data()), EEPROM_SIZE);
        std::cout << "[SaveSystem] Native game save successfully loaded from: " << path << std::endl;
        return true;
    }

    // 2. Guardar archivo físico en AppData cuando el juego escribe en la EEPROM
    bool saveEEPROM() {
        std::string path = PathManager::getSaveFilePath();
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[SaveSystem] Error writing native save: " << path << std::endl;
            return false;
        }

        file.write(reinterpret_cast<const char*>(eepromData.data()), EEPROM_SIZE);
        std::cout << "[SaveSystem] Native game progress saved to: " << path << std::endl;
        return true;
    }

    // --- HOOKS OFICIALES DE LIBULTRA (osEepromRead / osEepromWrite) ---

    // Hook: osEepromRead (El juego lee un bloque de guardado de 8 bytes)
    int32_t osEepromRead(uint8_t address, uint8_t* buffer) {
        if (address >= EEPROM_NUM_BLOCKS || !buffer) return -1;
        std::memcpy(buffer, &eepromData[address * EEPROM_BLOCK_SIZE], EEPROM_BLOCK_SIZE);
        return 0; // SUCCESS
    }

    // Hook: osEepromWrite (El juego guarda progreso oficial en el cartucho)
    int32_t osEepromWrite(uint8_t address, const uint8_t* buffer) {
        if (address >= EEPROM_NUM_BLOCKS || !buffer) return -1;
        std::memcpy(&eepromData[address * EEPROM_BLOCK_SIZE], buffer, EEPROM_BLOCK_SIZE);
        
        // Guardar automáticamente a disco en segundo plano
        saveEEPROM();
        return 0; // SUCCESS
    }

    // Hook: osEepromProbe (El juego comprueba qué chip de guardado tiene el cartucho)
    // 0x02 = 16Kbit EEPROM (Conker's Bad Fur Day)
    int32_t osEepromProbe() {
        return 0x02;
    }

    uint8_t* getEEPROMData() { return eepromData.data(); }

private:
    SaveSystem() = default;
    std::vector<uint8_t> eepromData;
};

} // namespace N64
