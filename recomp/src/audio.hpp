#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <mutex>
#include <cmath>
#include <iostream>
#include <SDL.h>
#include "memory.hpp"

namespace N64 {

// Estructura de un buffer de Audio AI de Nintendo 64
struct AIBuffer {
    std::vector<int16_t> samples;
    uint32_t frequency;
};

class AudioManager {
public:
    static AudioManager& getInstance() {
        static AudioManager instance;
        return instance;
    }

    bool init(int sampleRate = 44100) {
        frequency = sampleRate;
        audioQueue.clear();

        SDL_AudioSpec wantedSpec;
        SDL_zero(wantedSpec);
        wantedSpec.freq = sampleRate;
        wantedSpec.format = AUDIO_S16SYS; // 16-bit PCM firmado nativo
        wantedSpec.channels = 2;          // Stereo
        wantedSpec.samples = 1024;
        wantedSpec.callback = audioCallbackStatic;
        wantedSpec.userdata = this;

        audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wantedSpec, &obtainedSpec, 0);
        if (audioDevice == 0) {
            std::cerr << "[Audio] Warning: Failed to open SDL Audio Device: " << SDL_GetError() << std::endl;
            return false;
        }

        SDL_PauseAudioDevice(audioDevice, 0); // Iniciar playback activo
        std::cout << "[Audio] N64 Audio Interface (AI) & Synthesizer initialized at " 
                  << obtainedSpec.freq << " Hz (Stereo 16-bit PCM, Device ID: " << audioDevice << ")" << std::endl;
        
        // playBootJingle(); -- Silenciado para arranque limpio
        return true;
    }

    void shutdown() {
        if (audioDevice != 0) {
            SDL_CloseAudioDevice(audioDevice);
            audioDevice = 0;
        }
    }

    void setFrequency(uint32_t freq) { frequency = freq; }

    void queueAudioBuffer(uint32_t bufferVaddr, uint32_t size) {
        queueAIDMA(bufferVaddr, size, frequency);
    }

    // Procesa una transferencia DMA de audio desde RDRAM al hardware AI de N64 (osAiSetNextBuffer)
    void queueAIDMA(uint32_t rdramVaddr, size_t lengthInBytes, uint32_t freq = 44100) {
        if (lengthInBytes == 0 || audioDevice == 0) return;

        uint8_t* rdram = Memory::getInstance().getRDRAM();
        uint32_t paddr = Memory::getInstance().toPhysical(rdramVaddr);

        if (paddr + lengthInBytes > Memory::getInstance().getRDRAMSize()) return;

        size_t sampleCount = lengthInBytes / sizeof(int16_t);
        std::vector<int16_t> pcmSamples(sampleCount);

        const int16_t* src = reinterpret_cast<const int16_t*>(rdram + paddr);
        for (size_t i = 0; i < sampleCount; ++i) {
            // Conversión Big-Endian a Little-Endian de audio N64
            uint16_t raw = static_cast<uint16_t>(src[i]);
            raw = static_cast<uint16_t>((raw >> 8) | (raw << 8));
            pcmSamples[i] = static_cast<int16_t>(raw);
        }

        std::lock_guard<std::mutex> lock(audioMutex);
        audioQueue.push_back({std::move(pcmSamples), freq});
    }

    // Melodía de intro polifónica / test jingle
    void playBootJingle() {
        std::vector<int16_t> jingleSamples;
        int sampleRate = (obtainedSpec.freq > 0) ? obtainedSpec.freq : 44100;
        
        // Acorde nostálgico de N64 (C4, E4, G4, C5)
        float notes[] = { 261.63f, 329.63f, 392.00f, 523.25f, 659.25f };
        int noteDuration = sampleRate * 0.14f;

        for (float freqHz : notes) {
            for (int i = 0; i < noteDuration; ++i) {
                float t = static_cast<float>(i) / sampleRate;
                float env = 1.0f - (static_cast<float>(i) / noteDuration);
                int16_t sample = static_cast<int16_t>(std::sin(2.0f * 3.14159265f * freqHz * t) * 6000.0f * env);
                jingleSamples.push_back(sample); // Izquierda
                jingleSamples.push_back(sample); // Derecha
            }
        }

        std::lock_guard<std::mutex> lock(audioMutex);
        audioQueue.push_back({std::move(jingleSamples), static_cast<uint32_t>(sampleRate)});
        std::cout << "[Audio] N64 Rareware Audio Stream queued to playback buffer!" << std::endl;
    }

private:
    AudioManager() : audioDevice(0), frequency(44100) {}
    SDL_AudioDeviceID audioDevice;
    SDL_AudioSpec obtainedSpec{};
    int frequency;

    std::mutex audioMutex;
    std::vector<AIBuffer> audioQueue;

    static void audioCallbackStatic(void* userdata, Uint8* stream, int len) {
        reinterpret_cast<AudioManager*>(userdata)->audioCallback(stream, len);
    }

    void audioCallback(Uint8* stream, int len) {
        std::memset(stream, 0, len); // Silencio por defecto
        std::lock_guard<std::mutex> lock(audioMutex);

        if (audioQueue.empty()) return;

        int bytesNeeded = len;
        int streamOffset = 0;

        while (bytesNeeded > 0 && !audioQueue.empty()) {
            auto& front = audioQueue.front();
            int availableBytes = static_cast<int>(front.samples.size() * sizeof(int16_t));

            if (availableBytes <= bytesNeeded) {
                std::memcpy(stream + streamOffset, front.samples.data(), availableBytes);
                streamOffset += availableBytes;
                bytesNeeded -= availableBytes;
                audioQueue.erase(audioQueue.begin());
            } else {
                std::memcpy(stream + streamOffset, front.samples.data(), bytesNeeded);
                size_t samplesConsumed = bytesNeeded / sizeof(int16_t);
                front.samples.erase(front.samples.begin(), front.samples.begin() + samplesConsumed);
                bytesNeeded = 0;
            }
        }
    }
};

} // namespace N64
