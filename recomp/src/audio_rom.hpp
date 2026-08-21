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

// Indices identificados cruzando el tamano en bytes de cada archivo de
// exported_assets/audio con las entradas de la tabla de assets16; el orden
// coincide con el de parseTracks().
//
// Importante: el track 0 es `sfx_conker_grunt`, un efecto de sonido. Arrancar
// reproduciendo el track 0 hacia que el juego empezara con un gruñido en vez
// de con la musica de Rareware.
namespace MusicTrack {
    constexpr size_t WINDY_THEME     = 5;
    constexpr size_t BAT_TOWER       = 7;
    constexpr size_t HUNGOVER_THEME  = 13;
    constexpr size_t IT_IS_WAR       = 20;
    constexpr size_t POO_MOUNTAIN    = 33;
    constexpr size_t UGA_BUGA        = 82;
    constexpr size_t SPOOKY          = 200;
    constexpr size_t OVERWORLD       = 280;
    constexpr size_t CONKER_THEME    = 283;
    constexpr size_t SLOPRANO        = 336;
    constexpr size_t TITLE_RAREWARE  = 390;
}

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

    // Decodifica un track completo a PCM 16-bit EN SU FORMATO NATIVO.
    //
    // Los tracks de la ROM no son 44100/estereo: el track 0, por ejemplo, es
    // mono a 22050 Hz. Devolvemos hz/canales reales para que el reproductor
    // pueda convertirlos; encolarlos crudos en el dispositivo de 44100 estereo
    // los reproducia a ~4x velocidad.
    std::vector<int16_t> decodeTrack(size_t idx, int& outHz, int& outChannels) {
        outHz = 0;
        outChannels = 0;
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
        uint32_t frames = 0, resyncs = 0;
        while (consumed < mp3Len) {
            uint32_t remaining = mp3Len - consumed;
            if (remaining < 4) break;

            mp3dec_frame_info_t info{};
            int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];

            int samples = mp3dec_decode_frame(&dec, mp3Buf + consumed, remaining, pcm, &info);

            if (info.frame_bytes == 0) {
                // Los streams de la ROM llevan bytes de relleno intercalados
                // entre frames. Rendirse al primero truncaba las pistas: la
                // musica del logo de Rareware daba 1.25s de sus 7.2s reales.
                // Buscamos la siguiente cabecera y continuamos.
                uint32_t next = consumed + 1;
                while (next + 1 < mp3Len &&
                       !(mp3Buf[next] == 0xFF && (mp3Buf[next + 1] & 0xE0) == 0xE0)) {
                    ++next;
                }
                if (next + 1 >= mp3Len) break;
                consumed = next;
                ++resyncs;
                continue;
            }

            consumed += info.frame_bytes;
            ++frames;

            if (samples > 0) {
                if (outHz == 0) {
                    outHz = info.hz;
                    outChannels = info.channels;
                }
                int totalSamples = samples * info.channels;
                output.insert(output.end(), pcm, pcm + totalSamples);
            }
        }

        std::cout << "[ROMaudio]   decode: " << frames << " frames, " << resyncs
                  << " resyncs, consumed " << consumed << "/" << mp3Len << " bytes" << std::endl;

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

    void init(SDL_AudioDeviceID device, const SDL_AudioSpec& obtainedSpec) {
        audioDevice = device;
        deviceSpec  = obtainedSpec;
    }

    // Reproduce un track de la ROM, convirtiendolo al formato del dispositivo.
    void playTrack(size_t idx) {
        if (!audioDevice) return;

        int srcHz = 0, srcCh = 0;
        auto pcm = ROMaudioDecoder::getInstance().decodeTrack(idx, srcHz, srcCh);
        if (pcm.empty() || srcHz <= 0 || srcCh <= 0) {
            std::cerr << "[ROMaudio] Track " << idx << " could not be decoded." << std::endl;
            return;
        }

        double seconds = static_cast<double>(pcm.size()) / (srcHz * srcCh);
        std::cout << "[ROMaudio] Track " << idx << ": " << srcHz << " Hz, " << srcCh
                  << "ch, " << seconds << "s -> device " << deviceSpec.freq << " Hz, "
                  << static_cast<int>(deviceSpec.channels) << "ch" << std::endl;

        const Uint32 srcBytes = static_cast<Uint32>(pcm.size() * sizeof(int16_t));

        // Ya coincide con el dispositivo: sin conversion.
        if (srcHz == deviceSpec.freq && srcCh == deviceSpec.channels) {
            SDL_ClearQueuedAudio(audioDevice);
            SDL_QueueAudio(audioDevice, pcm.data(), srcBytes);
            SDL_PauseAudioDevice(audioDevice, 0);
            return;
        }

        // Resampleo + conversion de canales. Sin esto, un track mono de 22050 Hz
        // encolado en un dispositivo estereo de 44100 suena a ~4x velocidad.
        SDL_AudioStream* stream = SDL_NewAudioStream(
            AUDIO_S16SYS, static_cast<Uint8>(srcCh), srcHz,
            deviceSpec.format, deviceSpec.channels, deviceSpec.freq);
        if (!stream) {
            std::cerr << "[ROMaudio] SDL_NewAudioStream failed: " << SDL_GetError() << std::endl;
            return;
        }

        if (SDL_AudioStreamPut(stream, pcm.data(), srcBytes) != 0) {
            std::cerr << "[ROMaudio] SDL_AudioStreamPut failed: " << SDL_GetError() << std::endl;
            SDL_FreeAudioStream(stream);
            return;
        }
        SDL_AudioStreamFlush(stream);

        int available = SDL_AudioStreamAvailable(stream);
        if (available <= 0) {
            SDL_FreeAudioStream(stream);
            return;
        }

        std::vector<uint8_t> converted(static_cast<size_t>(available));
        int got = SDL_AudioStreamGet(stream, converted.data(), available);
        SDL_FreeAudioStream(stream);
        if (got <= 0) return;

        SDL_ClearQueuedAudio(audioDevice);
        SDL_QueueAudio(audioDevice, converted.data(), static_cast<Uint32>(got));
        SDL_PauseAudioDevice(audioDevice, 0);
    }

    void stop() {
        if (audioDevice) SDL_ClearQueuedAudio(audioDevice);
    }

private:
    ROMaudioPlayer() : audioDevice(0) {}
    SDL_AudioDeviceID audioDevice;
    SDL_AudioSpec deviceSpec{};
};

} // namespace N64
