#pragma once
#include <Arduino.h>
#include <FS.h>
#include <Preferences.h>
#include <esp_heap_caps.h>
#include "wake_word_detector.h"

// Verwaltet microWakeWord-Modelle (*.tflite + gleichnamiges *.json) im SD-Ordner /wakeword
class WakeWordManager {
public:
    static constexpr const char *FOLDER = "/wakeword";

    void begin(fs::FS *fs) {
        fs_ = fs;
        prefs_.begin("wakeword", true);
        selected_ = prefs_.getString("model", "");
        endOfSpeechSilenceMs_ = prefs_.getUShort("silence_ms", 1000);
        prefs_.end();
        ensureFolder();
        Serial.printf("[WAKE] Ordner %s: %s, Auswahl: %s\n", FOLDER,
                      folderReady_ ? "bereit" : "nicht verfuegbar",
                      selected_.isEmpty() ? "-" : selected_.c_str());
    }

    const String &selected() const { return selected_; }
    uint16_t endOfSpeechSilenceMs() const { return endOfSpeechSilenceMs_; }

    bool selectedAvailable() {
        return !selected_.isEmpty() && fs_ && fs_->exists(modelPath(selected_));
    }

    bool select(const String &name) {
        if (!name.isEmpty() && (!isValidName(name) || !fs_ || !fs_->exists(modelPath(name)))) return false;
        selected_ = name;
        prefs_.begin("wakeword", false);
        prefs_.putString("model", selected_);
        prefs_.end();
        Serial.printf("[WAKE] Modell gewaehlt: %s\n", selected_.isEmpty() ? "-" : selected_.c_str());
        return true;
    }

    // Laedt das gewaehlte Modell (16-Byte-ausgerichtet, PSRAM) und die Parameter aus der .json
    bool loadSelected(uint8_t *&model, WakeWordDetector::Settings &settings, String &error) {
        model = nullptr;
        if (selected_.isEmpty()) { error = "Kein Wake Word gewaehlt"; return false; }
        if (!fs_) { error = "Keine SD-Karte"; return false; }

        File file = fs_->open(modelPath(selected_));
        if (!file || file.isDirectory()) { error = "Modelldatei fehlt"; return false; }
        const size_t size = file.size();
        if (size == 0 || size > MAX_MODEL_SIZE) { file.close(); error = "Modelldatei hat ungueltige Groesse"; return false; }

        model = static_cast<uint8_t *>(heap_caps_aligned_alloc(16, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        if (!model) { file.close(); error = "Kein Speicher fuer Modell"; return false; }
        size_t offset = 0;
        while (offset < size) {
            const int got = file.read(model + offset, size - offset);
            if (got <= 0) break;
            offset += got;
        }
        file.close();
        if (offset != size) {
            heap_caps_free(model);
            model = nullptr;
            error = "Modelldatei konnte nicht gelesen werden";
            return false;
        }

        const String manifest = readManifest(selected_);
        const String cutoff = numberField(manifest, "probability_cutoff");
        const String window = numberField(manifest, "sliding_window_size");
        const String arena = numberField(manifest, "tensor_arena_size");
        const String step = numberField(manifest, "feature_step_size");
        if (cutoff.length()) settings.probabilityCutoff = cutoff.toFloat();
        if (window.length()) settings.slidingWindowSize = constrain(window.toInt(), 1, 32);
        if (arena.length()) settings.tensorArenaSize = constrain(arena.toInt(), 8000, 512000);
        if (step.length()) settings.featureStepMs = constrain(step.toInt(), 1, 40);
        settings.endOfSpeechSilenceMs = endOfSpeechSilenceMs_;
        Serial.printf("[WAKE] Modell %s geladen (%u Bytes)\n", selected_.c_str(), (unsigned)size);
        return true;
    }

    bool saveSelectedSettings(float cutoff, int window, uint16_t endOfSpeechSilenceMs, String &error) {
        if (selected_.isEmpty()) { error = "Kein Wake Word gewaehlt"; return false; }
        if (!fs_) { error = "Keine SD-Karte"; return false; }
        if (!isfinite(cutoff) || cutoff < 0.0f || cutoff > 1.0f || window < 1 || window > 32 ||
            endOfSpeechSilenceMs > 3000 || endOfSpeechSilenceMs % 100 != 0) {
            error = "Ungueltige Einstellungen";
            return false;
        }

        const String path = manifestPath(selected_);
        String manifest = readManifest(selected_);
        if (manifest.isEmpty()) { error = "Modell-JSON fehlt oder ist ungueltig"; return false; }
        if (!replaceNumberField(manifest, "probability_cutoff", String(cutoff, 2)) ||
            !replaceNumberField(manifest, "sliding_window_size", String(window))) {
            error = "Einstellungen fehlen in der Modell-JSON";
            return false;
        }

        const String temporaryPath = path + ".tmp";
        const String backupPath = path + ".bak";
        if (fs_->exists(temporaryPath)) fs_->remove(temporaryPath);
        if (fs_->exists(backupPath)) fs_->remove(backupPath);

        File temporary = fs_->open(temporaryPath, "w");
        if (!temporary) { error = "Temporaere JSON konnte nicht erstellt werden"; return false; }
        const size_t written = temporary.write(reinterpret_cast<const uint8_t *>(manifest.c_str()), manifest.length());
        temporary.close();
        if (written != manifest.length()) {
            fs_->remove(temporaryPath);
            error = "JSON konnte nicht vollstaendig geschrieben werden";
            return false;
        }

        if (!fs_->rename(path, backupPath)) {
            fs_->remove(temporaryPath);
            error = "Original-JSON konnte nicht gesichert werden";
            return false;
        }
        if (!fs_->rename(temporaryPath, path)) {
            fs_->rename(backupPath, path);
            fs_->remove(temporaryPath);
            error = "Neue JSON konnte nicht aktiviert werden";
            return false;
        }
        fs_->remove(backupPath);
        prefs_.begin("wakeword", false);
        const size_t saved = prefs_.putUShort("silence_ms", endOfSpeechSilenceMs);
        prefs_.end();
        if (saved != sizeof(endOfSpeechSilenceMs)) {
            error = "Stillezeit konnte nicht gespeichert werden";
            return false;
        }
        endOfSpeechSilenceMs_ = endOfSpeechSilenceMs;
        Serial.printf("[WAKE] Einstellungen fuer %s gespeichert: Schwelle %.2f, Fenster %d\n",
                      selected_.c_str(), cutoff, window);
        return true;
    }

    String listJson() {
        ensureFolder();
        String json = "{\"sd\":" + String(fs_ ? "true" : "false") +
                      ",\"folder\":" + String(folderReady_ ? "true" : "false") +
                      ",\"path\":\"" + String(FOLDER) + "\"" +
                      ",\"selected\":\"" + jsonEscape(selected_) + "\"" +
                      ",\"end_of_speech_silence_ms\":" + String(endOfSpeechSilenceMs_) +
                      ",\"models\":[";
        if (folderReady_) {
            File dir = fs_->open(FOLDER);
            bool first = true;
            while (dir) {
                File file = dir.openNextFile();
                if (!file) break;
                String fileName = baseName(file.name());
                const size_t size = file.size();
                const bool isDir = file.isDirectory();
                file.close();
                if (isDir || !fileName.endsWith(".tflite")) continue;

                String name = fileName.substring(0, fileName.length() - 7);
                if (!isValidName(name)) continue;
                String manifest = readManifest(name);
                if (!first) json += ',';
                first = false;
                json += "{\"name\":\"" + jsonEscape(name) + "\"" +
                        ",\"size\":" + String((uint32_t)size) +
                        ",\"manifest\":" + String(manifest.isEmpty() ? "false" : "true") +
                        ",\"wake_word\":\"" + jsonEscape(stringField(manifest, "wake_word")) + "\"" +
                        ",\"cutoff\":\"" + jsonEscape(numberField(manifest, "probability_cutoff")) + "\"" +
                                                ",\"window\":\"" + jsonEscape(numberField(manifest, "sliding_window_size")) + "\"}";
            }
            if (dir) dir.close();
        }
        json += "]}";
        return json;
    }

private:
    static constexpr size_t MAX_MODEL_SIZE = 1024 * 1024;

    fs::FS *fs_ = nullptr;
    Preferences prefs_;
    String selected_;
    bool folderReady_ = false;
    uint16_t endOfSpeechSilenceMs_ = 1000;

    void ensureFolder() {
        folderReady_ = false;
        if (!fs_) return;
        if (!fs_->exists(FOLDER) && fs_->mkdir(FOLDER)) {
            Serial.printf("[WAKE] Ordner %s angelegt.\n", FOLDER);
        }
        File dir = fs_->open(FOLDER);
        folderReady_ = dir && dir.isDirectory();
        if (dir) dir.close();
    }

    static String modelPath(const String &name) { return String(FOLDER) + "/" + name + ".tflite"; }
    static String manifestPath(const String &name) { return String(FOLDER) + "/" + name + ".json"; }

    static String baseName(const char *path) {
        String value(path);
        const int slash = value.lastIndexOf('/');
        return slash >= 0 ? value.substring(slash + 1) : value;
    }

    // Nur einfache Dateinamen zulassen, damit kein Zugriff ausserhalb von /wakeword moeglich ist
    static bool isValidName(const String &name) {
        if (name.isEmpty() || name.length() > 64 || name[0] == '.') return false;
        for (size_t i = 0; i < name.length(); ++i) {
            const char c = name[i];
            if (!isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-' && c != '.') return false;
        }
        return true;
    }

    String readManifest(const String &name) {
        File file = fs_->open(manifestPath(name));
        if (!file || file.isDirectory() || file.size() > 4096) {
            if (file) file.close();
            return "";
        }
        String content = file.readString();
        file.close();
        return content;
    }

    static int valueStart(const String &json, const char *key) {
        const int keyPos = json.indexOf(String("\"") + key + "\"");
        if (keyPos < 0) return -1;
        const int colon = json.indexOf(':', keyPos);
        if (colon < 0) return -1;
        int pos = colon + 1;
        while (pos < (int)json.length() && isspace(static_cast<unsigned char>(json[pos]))) ++pos;
        return pos;
    }

    static String stringField(const String &json, const char *key) {
        const int start = valueStart(json, key);
        if (start < 0 || json[start] != '"') return "";
        const int end = json.indexOf('"', start + 1);
        return end > start ? json.substring(start + 1, end) : "";
    }

    static String numberField(const String &json, const char *key) {
        const int start = valueStart(json, key);
        if (start < 0) return "";
        int end = start;
        while (end < (int)json.length() && (isdigit(static_cast<unsigned char>(json[end])) || json[end] == '.' || json[end] == '-')) ++end;
        return json.substring(start, end);
    }

    static bool replaceNumberField(String &json, const char *key, const String &replacement) {
        const int start = valueStart(json, key);
        if (start < 0 || !(isdigit(static_cast<unsigned char>(json[start])) || json[start] == '-')) return false;
        int end = start;
        while (end < (int)json.length()) {
            const char c = json[end];
            if (!isdigit(static_cast<unsigned char>(c)) && c != '.' && c != '-' && c != '+' && c != 'e' && c != 'E') break;
            ++end;
        }
        if (end == start) return false;
        json = json.substring(0, start) + replacement + json.substring(end);
        return true;
    }

    static String jsonEscape(const String &value) {
        String result;
        for (size_t i = 0; i < value.length(); ++i) {
            const char c = value[i];
            if (c == '\\' || c == '"') result += '\\';
            if (static_cast<unsigned char>(c) < 0x20) continue;
            result += c;
        }
        return result;
    }
};

extern WakeWordManager wakeWordMgr;
