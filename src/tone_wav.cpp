#include "tone_wav.h"
#include <math.h>
#include <vector>

namespace {

constexpr uint32_t SAMPLE_RATE = 44100;
constexpr uint32_t MAX_TOTAL_MS = 5000;
constexpr size_t MAX_TONES = 64;
constexpr float AMPLITUDE = 4500.0f;
// Kurzes Ein-/Ausblenden verhindert Knacken an den Tongrenzen
constexpr uint32_t FADE_SAMPLES = SAMPLE_RATE * 5 / 1000;
constexpr size_t BUFFER_SAMPLES = 256;

enum class Wave { Sine, Square, Triangle, Sawtooth };

struct Tone {
    float freq;
    uint32_t samples;
    Wave wave;
};

void writeLe16(uint8_t *dst, uint16_t value) {
    dst[0] = value & 0xff;
    dst[1] = (value >> 8) & 0xff;
}

void writeLe32(uint8_t *dst, uint32_t value) {
    dst[0] = value & 0xff;
    dst[1] = (value >> 8) & 0xff;
    dst[2] = (value >> 16) & 0xff;
    dst[3] = (value >> 24) & 0xff;
}

bool parseWave(const char *type, Wave &wave) {
    if (strcmp(type, "sine") == 0) wave = Wave::Sine;
    else if (strcmp(type, "square") == 0) wave = Wave::Square;
    else if (strcmp(type, "triangle") == 0) wave = Wave::Triangle;
    else if (strcmp(type, "sawtooth") == 0) wave = Wave::Sawtooth;
    else return false;
    return true;
}

// phase im Bereich [0, 1); Rechteck/Saegezahn leiser, da sie lauter wirken
float waveValue(Wave wave, float phase) {
    switch (wave) {
        case Wave::Square: return (phase < 0.5f ? 1.0f : -1.0f) * 0.6f;
        case Wave::Triangle: return 1.0f - 4.0f * fabsf(phase - 0.5f);
        case Wave::Sawtooth: return (2.0f * phase - 1.0f) * 0.6f;
        default: return sinf(2.0f * PI * phase);
    }
}

}  // namespace

void makeWavHeader(uint8_t header[44], uint32_t sampleCount, uint32_t sampleRate) {
    const uint32_t dataBytes = sampleCount * sizeof(int16_t);
    memset(header, 0, 44);
    memcpy(header, "RIFF", 4);
    writeLe32(header + 4, 36 + dataBytes);
    memcpy(header + 8, "WAVEfmt ", 8);
    writeLe32(header + 16, 16);
    writeLe16(header + 20, 1);
    writeLe16(header + 22, 1);
    writeLe32(header + 24, sampleRate);
    writeLe32(header + 28, sampleRate * sizeof(int16_t));
    writeLe16(header + 32, sizeof(int16_t));
    writeLe16(header + 34, 16);
    memcpy(header + 36, "data", 4);
    writeLe32(header + 40, dataBytes);
}

bool writeToneWav(fs::FS &filesystem, const char *path, JsonArrayConst tones, String &error) {
    if (tones.isNull() || tones.size() == 0 || tones.size() > MAX_TONES) {
        error = "Ton-Sequenz muss ein Array mit 1 bis " + String((unsigned)MAX_TONES) + " Eintraegen sein";
        return false;
    }

    std::vector<Tone> parsed;
    parsed.reserve(tones.size());
    uint32_t totalMs = 0;
    uint32_t totalSamples = 0;
    for (JsonObjectConst entry : tones) {
        const float freq = entry["freq"] | -1.0f;
        const long duration = entry["duration"] | 0L;
        Wave wave;
        if (freq < 0.0f || freq >= SAMPLE_RATE / 2.0f || duration <= 0 || !parseWave(entry["type"] | "sine", wave)) {
            error = "Ungueltiger Ton-Eintrag " + String((unsigned)parsed.size() + 1);
            return false;
        }
        totalMs += duration;
        if (totalMs > MAX_TOTAL_MS) {
            error = "Ton-Sequenz laenger als " + String(MAX_TOTAL_MS) + " ms";
            return false;
        }
        const uint32_t samples = SAMPLE_RATE * (uint32_t)duration / 1000;
        parsed.push_back({freq, samples, wave});
        totalSamples += samples;
    }

    File file = filesystem.open(path, "w");
    if (!file) {
        error = String("Datei konnte nicht erstellt werden: ") + path;
        return false;
    }
    uint8_t header[44];
    makeWavHeader(header, totalSamples, SAMPLE_RATE);
    bool ok = file.write(header, sizeof(header)) == sizeof(header);

    int16_t buffer[BUFFER_SAMPLES];
    size_t fill = 0;
    for (const Tone &tone : parsed) {
        const float step = tone.freq / SAMPLE_RATE;
        const uint32_t fade = min(FADE_SAMPLES, tone.samples / 2);
        float phase = 0.0f;
        for (uint32_t i = 0; ok && i < tone.samples; ++i) {
            float gain = 1.0f;
            if (fade > 0) gain = fminf(1.0f, fminf((float)i / fade, (float)(tone.samples - 1 - i) / fade));
            buffer[fill++] = (int16_t)(waveValue(tone.wave, phase) * AMPLITUDE * gain);
            phase += step;
            if (phase >= 1.0f) phase -= 1.0f;
            if (fill == BUFFER_SAMPLES) {
                ok = file.write(reinterpret_cast<const uint8_t *>(buffer), sizeof(buffer)) == sizeof(buffer);
                fill = 0;
            }
        }
    }
    if (ok && fill > 0) {
        const size_t bytes = fill * sizeof(int16_t);
        ok = file.write(reinterpret_cast<const uint8_t *>(buffer), bytes) == bytes;
    }
    file.close();
    if (!ok) {
        filesystem.remove(path);
        error = String("Schreiben fehlgeschlagen: ") + path;
    }
    return ok;
}
