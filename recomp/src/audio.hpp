#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <cmath>
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
        desiredSpec.format = AUDIO_S16SYS; // PCM 16-bit nativo del sistema
        desiredSpec.channels = 2;          // Estéreo
        desiredSpec.samples = 1024;
        desiredSpec.callback = nullptr;

        audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);
        if (audioDevice == 0) {
            std::cerr << "[Audio] Failed to open SDL Audio Device: " << SDL_GetError() << std::endl;
            return false;
        }

        SDL_PauseAudioDevice(audioDevice, 0); // Iniciar salida de audio
        std::cout << "[Audio] Audio system initialized at " << obtainedSpec.freq << " Hz (Stereo 16-bit PCM)" << std::endl;
        
        // Generar tono musical de prueba inicial (440Hz / Tono de arranque)
        playBootJingle();
        return true;
    }

    void shutdown() {
        if (audioDevice != 0) {
            SDL_CloseAudioDevice(audioDevice);
            audioDevice = 0;
        }
    }

    // Tono audible claro para confirmar funcionamiento de altavoces
    void playBootJingle() {
        if (audioDevice == 0) return;

        const int numSamples = sampleRate / 2; // 0.5 segundos de sonido
        std::vector<int16_t> buffer(numSamples * 2);

        for (int i = 0; i < numSamples; ++i) {
            // Frecuencia que sube suavemente (arpegio de arranque N64)
            float freq = 440.0f + (static_cast<float>(i) / numSamples) * 440.0f;
            float t = static_cast<float>(i) / sampleRate;
            int16_t sample = static_cast<int16_t>(std::sin(2.0f * 3.14159265f * freq * t) * 6000.0f); // Volumen moderado

            buffer[i * 2]     = sample; // Canal Izquierdo
            buffer[i * 2 + 1] = sample; // Canal Derecho
        }

        SDL_QueueAudio(audioDevice, buffer.data(), static_cast<uint32_t>(buffer.size() * sizeof(int16_t)));
        std::cout << "[Audio] Test Jingle queued to speakers!" << std::endl;
    }

    // Hook oficial N64: osAiSetNextBuffer
    void queueAudioBuffer(uint32_t rdramVaddr, size_t sizeBytes) {
        if (audioDevice == 0 || sizeBytes == 0) return;

        uint8_t* rdram = Memory::getInstance().getRDRAM();
        uint32_t paddr = rdramVaddr & 0x1FFFFFFF;

        if (paddr + sizeBytes <= Memory::getInstance().getRDRAMSize()) {
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
