#include <iostream>
#include <cstdint>
#include "save_system.hpp"
#include "audio.hpp"

extern "C" {

void osInitialize(void) {
    std::cout << "[N64 OS] osInitialize called" << std::endl;
}

void osViSetMode(void* mode) {
    (void)mode;
}

void osViBlack(uint8_t active) {
    (void)active;
}

void osViSwapBuffer(void* vaddr) {
    (void)vaddr;
}

// --- HOOKS DE AUDIO N64 (osAi) ---

int32_t osAiSetFrequency(uint32_t frequency) {
    std::cout << "[N64 OS] osAiSetFrequency: Setting playback sample rate to " << frequency << " Hz" << std::endl;
    N64::AudioManager::getInstance().setFrequency(frequency);
    return frequency;
}

int32_t osAiSetNextBuffer(void* vaddr, uint32_t size) {
    uint32_t bufferVaddr = reinterpret_cast<uintptr_t>(vaddr);
    N64::AudioManager::getInstance().queueAudioBuffer(bufferVaddr, size);
    return 0; // SUCCESS
}

uint32_t osAiGetLength(void) {
    return 0;
}

// --- HOOKS DE GUARDADO NATIVO OFICIAL (osEeprom) ---

int32_t osEepromProbe(void* mq) {
    (void)mq;
    std::cout << "[N64 OS] osEepromProbe: Detecting Save Chip -> 16Kbit EEPROM (Conker's Bad Fur Day)" << std::endl;
    return N64::SaveSystem::getInstance().osEepromProbe();
}

int32_t osEepromRead(void* mq, uint8_t address, uint8_t* buffer) {
    (void)mq;
    return N64::SaveSystem::getInstance().osEepromRead(address, buffer);
}

int32_t osEepromWrite(void* mq, uint8_t address, const uint8_t* buffer) {
    (void)mq;
    return N64::SaveSystem::getInstance().osEepromWrite(address, buffer);
}

int32_t osEepromLongRead(void* mq, uint8_t address, uint8_t* buffer, int32_t length) {
    (void)mq;
    for (int i = 0; i < length / 8; ++i) {
        if (N64::SaveSystem::getInstance().osEepromRead(static_cast<uint8_t>(address + i), buffer + (i * 8)) != 0) {
            return -1;
        }
    }
    return 0;
}

int32_t osEepromLongWrite(void* mq, uint8_t address, const uint8_t* buffer, int32_t length) {
    (void)mq;
    for (int i = 0; i < length / 8; ++i) {
        if (N64::SaveSystem::getInstance().osEepromWrite(static_cast<uint8_t>(address + i), buffer + (i * 8)) != 0) {
            return -1;
        }
    }
    return 0;
}

}
