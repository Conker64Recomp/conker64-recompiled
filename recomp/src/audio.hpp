#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <SDL.h>
#include "memory.hpp"

namespace N64 {

class AudioManager {
public:
    static AudioManager& getInstance() {
        static AudioManager instance;
        return instance;
    }

    bool init(int frequency = 44100) {
        sampleRate = frequency;

        SDL_AudioSpec desiredSpec;
        SDL_zero(desiredSpec);
        desiredSpec.freq = sampleRate;
        desiredSpec.format = AUDIO_S16MSB; // N64 entrega PCM de 16-bit Big-Endian
        desiredSpec.channels = 2;          // Estéreo
        desiredSpec.samples = 1024;
        desiredSpec.callback = nullptr;   // Usamos cola directa con SDL_QueueAudio

        audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);
        if (audioDevice == 0) {
            std::cerr << "[Audio] Failed to open SDL Audio Device: " << SDL_GetError() << std::endl;
            return false;
        }

        SDL_PauseAudioDevice(audioDevice, 0); // Iniciar playback
        std::cout << "[Audio] N64 Audio Interface (AI) initialized at " << obtainedSpec.freq << " Hz (Stereo 16-bit PCM)" << std::endl;
        return true;
    }

    void shutdown() {
        if (audioDevice != 0) {
            SDL_CloseAudioDevice(audioDevice);
            audioDevice = 0;
        }
    }

    // Hook oficial de N64: osAiSetNextBuffer
    void queueAudioBuffer(uint32_t rdramVaddr, size_t sizeBytes) {
        if (audioDevice == 0 || sizeBytes == 0) return;

        uint8_t* rdram = Memory::getInstance().getRDRAM();
        uint32_t paddr = rdramVaddr & 0x1FFFFFFF;

        if (paddr + sizeBytes <= Memory::getInstance().getRDRAMSize()) {
            // Encolar buffer de sonido directamente a la tarjeta de sonido de PC
            SDL_QueueAudio(audioDevice, &rdram[paddr], static_cast<uint32_t>(sizeBytes));
        }
    }

    void setFrequency(int freq) {
        if (freq != sampleRate) {
            shutdown();
            init(freq);
        }
    }

private:
    AudioManager() : audioDevice(0), sampleRate(44100) {}
    SDL_AudioDeviceID audioDevice;
    SDL_AudioSpec obtainedSpec;
    int sampleRate;
};

} // namespace N64
