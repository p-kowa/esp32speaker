#pragma once
#include <Arduino.h>
#include <FS.h>
#include <algorithm>
#include <vector>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "mic_manager.h"
#include "tone_wav.h"

// Manuelle Trainingsaufnahmen: rohes 16-kHz-PCM vom Mikrofon, unabhaengig vom Wake-Word-Detektor
class TrainingRecorder {
public:
    enum class State : uint8_t { Idle, Recording, Finished };

    static constexpr const char *FOLDER = "/wakeword/training";
    static constexpr size_t FIXED_SAMPLES = MicManager::SAMPLE_RATE * 2;
    static constexpr size_t MAX_SAMPLES = MicManager::SAMPLE_RATE * 10;
    static constexpr size_t MIN_SAMPLES = MicManager::SAMPLE_RATE / 4;

    State state() const { return state_; }
    bool fixedMode() const { return fixed_; }
    uint32_t elapsedMs() const { return samples_ * 1000UL / MicManager::SAMPLE_RATE; }
    uint32_t targetMs() const { return target_ * 1000UL / MicManager::SAMPLE_RATE; }

    // Das Mikrofon muss bereits laufen; der Detektor darf den Stream nicht gleichzeitig lesen
    bool start(MicManager &mic, bool fixed, String &error) {
        if (state_ != State::Idle) { error = "Aufnahme laeuft bereits"; return false; }
        size_t target = MAX_SAMPLES;
        if (fixed) target = FIXED_SAMPLES;
        buffer_ = static_cast<int16_t *>(heap_caps_malloc(target * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        if (!buffer_) { error = "Kein Speicher fuer Aufnahme"; return false; }

        mic_ = &mic;
        fixed_ = fixed;
        target_ = target;
        samples_ = 0;
        stopRequested_ = false;
        error_ = "";
        state_ = State::Recording;
        mic_->setStreamingEnabled(true);
        if (xTaskCreatePinnedToCore(taskEntry, "trainrec", 4096, this, 4, &task_, 0) != pdPASS) {
            mic_->setStreamingEnabled(false);
            task_ = nullptr;
            heap_caps_free(buffer_);
            buffer_ = nullptr;
            state_ = State::Idle;
            error = "Aufnahme-Task konnte nicht gestartet werden";
            return false;
        }
        return true;
    }

    void requestStop() { stopRequested_ = true; }

    // Schreibt die fertige Aufnahme als WAV; Aufrufer danach release() aufrufen
    bool save(fs::FS &fs, String &fileName, String &error) {
        if (state_ != State::Finished) { error = "Keine fertige Aufnahme"; return false; }
        if (error_[0]) { error = error_; return false; }
        if (samples_ < MIN_SAMPLES) { error = "Aufnahme zu kurz, verworfen"; return false; }
        if (!ensureFolder(fs)) { error = "Ordner konnte nicht angelegt werden"; return false; }

        const int index = nextIndex(fs);
        if (index > 9999) { error = "Maximal 9999 Aufnahmen"; return false; }
        char name[24];
        snprintf(name, sizeof(name), "sample_%04d.wav", index);
        const String path = String(FOLDER) + "/" + name;

        File file = fs.open(path, "w");
        if (!file) { error = "Datei konnte nicht erstellt werden"; return false; }
        uint8_t header[44];
        makeWavHeader(header, samples_, MicManager::SAMPLE_RATE);
        bool ok = file.write(header, sizeof(header)) == sizeof(header);
        const uint8_t *bytes = reinterpret_cast<const uint8_t *>(buffer_);
        const size_t total = samples_ * sizeof(int16_t);
        for (size_t offset = 0; ok && offset < total;) {
            const size_t chunk = std::min<size_t>(4096, total - offset);
            ok = file.write(bytes + offset, chunk) == chunk;
            offset += chunk;
        }
        file.close();
        if (!ok) {
            fs.remove(path);
            error = "Schreiben fehlgeschlagen (SD voll?)";
            return false;
        }
        fileName = name;
        return true;
    }

    void release() {
        if (buffer_) heap_caps_free(buffer_);
        buffer_ = nullptr;
        task_ = nullptr;
        state_ = State::Idle;
    }

    static bool validName(const String &name) {
        if (name.length() < 12 || name.length() > 20 || !name.startsWith("sample_") || !name.endsWith(".wav")) return false;
        for (size_t i = 7; i < name.length() - 4; ++i) {
            if (!isDigit(name[i])) return false;
        }
        return true;
    }

    static String listJson(fs::FS *fs) {
        String json = String("{\"sd\":") + (fs ? "true" : "false") + ",\"files\":[";
        if (fs && fs->exists(FOLDER)) {
            std::vector<std::pair<String, size_t>> files;
            File dir = fs->open(FOLDER);
            while (dir) {
                File file = dir.openNextFile();
                if (!file) break;
                String name = file.name();
                const int slash = name.lastIndexOf('/');
                if (slash >= 0) name = name.substring(slash + 1);
                if (!file.isDirectory() && validName(name)) files.emplace_back(name, file.size());
                file.close();
            }
            dir.close();
            std::sort(files.begin(), files.end(), [](const auto &a, const auto &b) { return a.first > b.first; });
            for (size_t i = 0; i < files.size(); ++i) {
                if (i) json += ',';
                json += "{\"name\":\"" + files[i].first + "\",\"size\":" + String((unsigned)files[i].second) + "}";
            }
        }
        json += "]}";
        return json;
    }

    const char *error() const { return error_; }

private:
    static void taskEntry(void *arg) { static_cast<TrainingRecorder *>(arg)->run(); }

    void run() {
        const size_t frameSamples = MicManager::FRAME_SAMPLES;
        uint32_t lastDataMs = millis();
        while (!stopRequested_ && samples_ < target_) {
            const size_t want = std::min<size_t>(frameSamples, target_ - samples_);
            const size_t got = mic_->read(buffer_ + samples_, want, pdMS_TO_TICKS(100));
            if (got) {
                samples_ += got;
                lastDataMs = millis();
            } else if (millis() - lastDataMs > 1000) {
                error_ = "Keine Mikrofondaten";
                break;
            }
        }
        mic_->setStreamingEnabled(false);
        task_ = nullptr;
        state_ = State::Finished;
        vTaskDelete(nullptr);
    }

    static bool ensureFolder(fs::FS &fs) {
        if (!fs.exists("/wakeword")) fs.mkdir("/wakeword");
        if (!fs.exists(FOLDER)) fs.mkdir(FOLDER);
        return fs.exists(FOLDER);
    }

    static int nextIndex(fs::FS &fs) {
        int highest = 0;
        File dir = fs.open(FOLDER);
        while (dir) {
            File file = dir.openNextFile();
            if (!file) break;
            String name = file.name();
            const int slash = name.lastIndexOf('/');
            if (slash >= 0) name = name.substring(slash + 1);
            if (!file.isDirectory() && validName(name)) highest = std::max(highest, (int)name.substring(7, name.length() - 4).toInt());
            file.close();
        }
        dir.close();
        return highest + 1;
    }

    MicManager *mic_ = nullptr;
    int16_t *buffer_ = nullptr;
    TaskHandle_t task_ = nullptr;
    volatile State state_ = State::Idle;
    volatile bool stopRequested_ = false;
    volatile size_t samples_ = 0;
    size_t target_ = 0;
    bool fixed_ = true;
    const char *error_ = "";
};
