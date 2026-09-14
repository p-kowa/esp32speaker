#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <vector>
#include "config.h"

class SdManager {
private:
    bool mounted = false;

    static String jsonEscape(const String &value) {
        String result;
        for (size_t i = 0; i < value.length(); ++i) {
            char c = value[i];
            if (c == '\\' || c == '"') result += '\\';
            result += c;
        }
        return result;
    }

    static bool isPlayable(const String &path) {
        String lower = path;
        lower.toLowerCase();
        return lower.endsWith(".mp3") || lower.endsWith(".wav") || lower.endsWith(".aac") || lower.endsWith(".flac");
    }

    void appendFiles(File directory, String &json) {
        while (true) {
            File file = directory.openNextFile();
            if (!file) break;
            if (file.isDirectory()) {
                appendFiles(file, json);
            } else {
                String path = file.path();
                if (isPlayable(path)) {
                    if (json.length() > 1) json += ',';
                    json += "{\"path\":\"" + jsonEscape(path) + "\",\"size\":" + String((uint32_t)file.size()) + "}";
                }
            }
            file.close();
        }
    }

public:
    bool begin() {
        SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        mounted = SD.begin(SD_CS, SPI, 20000000);
        if (mounted) {
            Serial.printf("[SD] Karte bereit, Typ=%d, Groesse=%llu MB\n", (int)SD.cardType(), SD.cardSize() / (1024ULL * 1024ULL));
        } else {
            Serial.println("[SD] Keine Karte gefunden oder Initialisierung fehlgeschlagen.");
        }
        return mounted;
    }

    bool isMounted() const { return mounted; }

    String filesJson() {
        if (!mounted) return "[]";
        File root = SD.open("/");
        String json = "[";
        appendFiles(root, json);
        root.close();
        json += ']';
        return json;
    }

    std::vector<String> paths() {
        std::vector<String> result;
        if (!mounted) return result;
        File root = SD.open("/");
        appendPaths(root, result);
        root.close();
        return result;
    }

    bool exists(const String &path) const {
        return mounted && SD.exists(path.c_str());
    }

    fs::FS &filesystem() { return SD; }

private:
    void appendPaths(File directory, std::vector<String> &result) {
        while (true) {
            File file = directory.openNextFile();
            if (!file) break;
            if (file.isDirectory()) {
                appendPaths(file, result);
            } else if (isPlayable(file.path())) {
                result.push_back(file.path());
            }
            file.close();
        }
    }
};

extern SdManager sdMgr;
