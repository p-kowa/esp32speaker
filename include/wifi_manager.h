#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

class WiFiConfigManager {
private:
    Preferences prefs;
    bool _isApMode = false;
    String _apSSID = "ESP32-Radio-Setup";

public:
    void begin() {
        // Initialisiert Preferences NVS Speicher
    }

    void loadCredentials(String &ssid, String &pass) {
        prefs.begin("wifi_config", true);
        ssid = prefs.getString("ssid", "");
        pass = prefs.getString("pass", "");
        prefs.end();
    }

    void saveCredentials(const String &ssid, const String &pass) {
        prefs.begin("wifi_config", false);
        prefs.putString("ssid", ssid);
        prefs.putString("pass", pass);
        prefs.end();
        Serial.printf("[WIFI-MGR] Zugangsdaten fuer '%s' im NVS gespeichert.\n", ssid.c_str());
    }

    void clearCredentials() {
        prefs.begin("wifi_config", false);
        prefs.clear();
        prefs.end();
        Serial.println("[WIFI-MGR] Zugangsdaten aus NVS geloescht.");
    }

    bool connectSTA(const String &ssid, const String &pass, uint32_t timeoutMs = 15000) {
        struct Candidate {
            uint8_t bssid[6];
            int32_t channel;
            int32_t rssi;
        };

        Candidate candidates[3] = {};
        uint8_t candidateCount = 0;

        Serial.printf("[WIFI-MGR] Suche staerkste APs fuer '%s'...\n", ssid.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);
        WiFi.setSleep(false);

        int16_t networkCount = WiFi.scanNetworks(false, true);
        for (int16_t i = 0; i < networkCount; ++i) {
            if (WiFi.SSID(i) != ssid || WiFi.BSSID(i) == nullptr) continue;

            bool duplicate = false;
            for (uint8_t j = 0; j < candidateCount; ++j) {
                if (memcmp(candidates[j].bssid, WiFi.BSSID(i), 6) == 0) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) continue;

            Candidate candidate = {};
            memcpy(candidate.bssid, WiFi.BSSID(i), 6);
            candidate.channel = WiFi.channel(i);
            candidate.rssi = WiFi.RSSI(i);

            uint8_t position = candidateCount < 3 ? candidateCount++ : 3;
            while (position > 0 && candidates[position - 1].rssi < candidate.rssi) {
                if (position < 3) candidates[position] = candidates[position - 1];
                --position;
            }
            if (position < 3) candidates[position] = candidate;
        }
        WiFi.scanDelete();

        unsigned long deadline = millis() + timeoutMs;
        for (uint8_t i = 0; i < candidateCount; ++i) {
            if (millis() >= deadline) break;
            Serial.printf("[WIFI-MGR] AP %u: Kanal %d, RSSI %d dBm\n",
                          i + 1, candidates[i].channel, candidates[i].rssi);
            WiFi.disconnect();
            WiFi.begin(ssid.c_str(), pass.c_str(), candidates[i].channel, candidates[i].bssid);

            unsigned long attemptStart = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - attemptStart < 5000) {
                delay(100);
            }
            if (WiFi.status() == WL_CONNECTED) {
                _isApMode = false;
                Serial.println("[WIFI-MGR] Verbunden (gezielter Mesh-AP)!");
                Serial.printf("[WIFI-MGR] IP: http://%s, RSSI: %d dBm\n",
                              WiFi.localIP().toString().c_str(), WiFi.RSSI());
                return true;
            }
        }

        if (millis() < deadline) {
            Serial.println("[WIFI-MGR] Automatische AP-Auswahl als Fallback...");
            WiFi.disconnect();
            WiFi.begin(ssid.c_str(), pass.c_str());
            while (WiFi.status() != WL_CONNECTED && millis() < deadline) {
                delay(100);
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
            _isApMode = false;
            Serial.println("[WIFI-MGR] Verbunden (automatische AP-Auswahl)!");
            Serial.printf("[WIFI-MGR] IP: http://%s, RSSI: %d dBm\n",
                          WiFi.localIP().toString().c_str(), WiFi.RSSI());
            return true;
        }

        Serial.println("[WIFI-MGR] Verbindung fehlerhaft.");
        return false;
    }

    void startAP() {
        _isApMode = true;
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(_apSSID.c_str());
        IPAddress apIP = WiFi.softAPIP();
        Serial.println("\n==========================================");
        Serial.println("  [WIFI-MGR] Hotspot Gestartet!");
        Serial.printf("  SSID: %s\n", _apSSID.c_str());
        Serial.printf("  IP:   http://%s\n", apIP.toString().c_str());
        Serial.println("==========================================");
    }

    // Fuehrt die Initialisierung aus
    bool initWifi(const String &fallbackSsid = "", const String &fallbackPass = "") {
        String savedSsid, savedPass;
        loadCredentials(savedSsid, savedPass);

        // Falls im NVS nichts ist, aber in config.h ein Fallback steht
        if (savedSsid.length() == 0 && fallbackSsid.length() > 0) {
            savedSsid = fallbackSsid;
            savedPass = fallbackPass;
        }

        if (savedSsid.length() > 0) {
            if (connectSTA(savedSsid, savedPass, 15000)) {
                return true;
            }
        }

        // Falls keine Daten da sind oder Verbindung schlug fehl -> AP starten
        startAP();
        return false;
    }

    bool isApMode() const {
        return _isApMode;
    }

    String scanNetworksJson() {
        Serial.println("[WIFI-MGR] Starte WLAN Scan...");
        int n = WiFi.scanNetworks();
        String json = "[";
        for (int i = 0; i < n; ++i) {
            if (i > 0) json += ",";
            json += "{";
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
            json += "\"auth\":" + String((int)WiFi.encryptionType(i));
            json += "}";
        }
        json += "]";
        WiFi.scanDelete();
        return json;
    }
};

extern WiFiConfigManager wifiMgr;
