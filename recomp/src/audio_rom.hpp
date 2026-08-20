#pragma once

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_ONLY_MP3
#include "minimp3.h"

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <SDL.h>

namespace N64 {

// Tabla de offsets de los 256 tracks de audio en assets16 de la ROM
struct AudioTrackEntry {
    uint32_t romOffset;   // Offset absoluto en la ROM
    uint32_t length;      // Tamaño en bytes del stream MP3
};

class ROMaudioDecoder {
public:
    static ROMaudioDecoder& getInstance() {
        static ROMaudioDecoder instance;
        return instance;
    }

    // Inicializa el decoder con la ROM en memoria
    void init(const uint8_t* romData, size_t romSize) {
        romPtr  = romData;
        romSz   = romSize;
        mp3dec_init(&mp3dec);
        parseTracks();
        std::cout << "[ROMaudio] Rareware MP3 Decoder ready. "
                  << tracks.size() << " audio tracks indexed from assets16." << std::endl;
    }

    // Devuelve el número de tracks disponibles
    size_t trackCount() const { return tracks.size(); }

    // Decodifica un track completo a PCM estéreo 16-bit 44100 Hz
    std::vector<int16_t> decodeTrack(size_t idx) {
        if (idx >= tracks.size() || !romPtr) return {};
        const auto& t = tracks[idx];
        if (t.romOffset + t.length > romSz || t.length < 4) return {};

        const uint8_t* mp3Buf  = romPtr + t.romOffset;
        uint32_t       mp3Len  = t.length;

        std::vector<int16_t> output;
        output.reserve(mp3Len * 2);

        mp3dec_t dec;
        mp3dec_init(&dec);

        uint32_t consumed = 0;
        while (consumed < mp3Len) {
            uint32_t remaining = mp3Len - consumed;
            if (remaining < 4) break;

            mp3dec_frame_info_t info{};
            int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];

            int samples = mp3dec_decode_frame(&dec, mp3Buf + consumed, remaining, pcm, &info);
            if (info.frame_bytes == 0) break; // No se pudo decodificar más
            consumed += info.frame_bytes;

            if (samples > 0) {
                int totalSamples = samples * info.channels;
                output.insert(output.end(), pcm, pcm + totalSamples);
            }
        }

        return output;
    }

    // Devuelve info del track sin decodificar completo
    bool getTrackInfo(size_t idx, int& hz, int& channels, int& kbps) {
        if (idx >= tracks.size() || !romPtr) return false;
        const auto& t = tracks[idx];
        if (t.romOffset + 512 > romSz) return false;

        mp3dec_t dec;
        mp3dec_init(&dec);
        mp3dec_frame_info_t info{};
        int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
        mp3dec_decode_frame(&dec, romPtr + t.romOffset, 512, pcm, &info);
        hz       = info.hz;
        channels = info.channels;
        kbps     = info.bitrate_kbps;
        return (info.hz > 0);
    }

private:
    ROMaudioDecoder() : romPtr(nullptr), romSz(0) {}
    const uint8_t* romPtr;
    size_t romSz;
    mp3dec_t mp3dec{};
    std::vector<AudioTrackEntry> tracks;

    // Parsea la tabla de offsets de assets16 (header de 8 bytes por entrada: [uncomp_offset][flags|length])
    void parseTracks() {
        tracks.clear();
        if (!romPtr || romSz < 0x01330478 + 2048) return;

        // assets16 empieza en 0x01330478
        constexpr uint32_t PKG_START = 0x01330478;
        const uint8_t* hdr = romPtr + PKG_START;

        for (int i = 0; i < 512; ++i) {
            uint32_t w0 = (static_cast<uint32_t>(hdr[i*8+0]) << 24) |
                          (static_cast<uint32_t>(hdr[i*8+1]) << 16) |
                          (static_cast<uint32_t>(hdr[i*8+2]) << 8)  |
                           static_cast<uint32_t>(hdr[i*8+3]);
            uint32_t w1 = (static_cast<uint32_t>(hdr[i*8+4]) << 24) |
                          (static_cast<uint32_t>(hdr[i*8+5]) << 16) |
                          (static_cast<uint32_t>(hdr[i*8+6]) << 8)  |
                           static_cast<uint32_t>(hdr[i*8+7]);

            if (w0 == 0 && w1 == 0) break;

            uint32_t absOffset = PKG_START + w0;
            uint32_t length    = w1 & 0x00FFFFFF;

            if (absOffset < romSz && length > 4 && absOffset + length <= romSz) {
                // Verificar sync frame MP3 (0xFF 0xEx o 0xFF 0xFx)
                if (romPtr[absOffset] == 0xFF && (romPtr[absOffset+1] & 0xE0) == 0xE0) {
                    tracks.push_back({ absOffset, length });
                }
            }
        }
    }
};

// Reproductor de audio de la ROM (integrado con SDL Audio Device)
class ROMaudioPlayer {
public:
    static ROMaudioPlayer& getInstance() {
        static ROMaudioPlayer instance;
        return instance;
    }

    void init(SDL_AudioDeviceID device) {
        audioDevice = device;
    }

    // Reproduce un track de la ROM directamente en el device SDL2
    void playTrack(size_t idx) {
        auto pcm = ROMaudioDecoder::getInstance().decodeTrack(idx);
        if (pcm.empty()) {
            std::cerr << "[ROMaudio] Track " << idx << " could not be decoded." << std::endl;
            return;
        }

        int hz = 44100, ch = 2, kbps = 0;
        ROMaudioDecoder::getInstance().getTrackInfo(idx, hz, ch, kbps);
        std::cout << "[ROMaudio] Playing Track " << idx << ": "
                  << hz << " Hz, " << ch << "ch, " << kbps
                  << " kbps, " << (pcm.size() / (hz * ch)) << "s" << std::endl;

        SDL_ClearQueuedAudio(audioDevice);
        SDL_QueueAudio(audioDevice, pcm.data(), static_cast<Uint32>(pcm.size() * sizeof(int16_t)));
        SDL_PauseAudioDevice(audioDevice, 0);
    }

    void stop() {
        if (audioDevice) SDL_ClearQueuedAudio(audioDevice);
    }

private:
    ROMaudioPlayer() : audioDevice(0) {}
    SDL_AudioDeviceID audioDevice;
};

} // namespace N64
