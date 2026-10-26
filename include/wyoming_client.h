#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WiFi.h>
#include <math.h>
#include "pcm_gain.h"

// Sends a recorded utterance to a Wyoming speech-to-text server (e.g. wyoming-faster-whisper) over TCP
class WyomingClient {
public:
    static constexpr uint16_t DEFAULT_PORT = 10300;

    void begin() {
        prefs.begin("wyoming", true);
        port_ = prefs.getUShort("port", DEFAULT_PORT);
        prefs.end();
    }

    uint16_t port() const { return port_; }

    void setPort(uint16_t newPort) {
        port_ = newPort ? newPort : static_cast<uint16_t>(DEFAULT_PORT);
        prefs.begin("wyoming", false);
        prefs.putUShort("port", port_);
        prefs.end();
    }

    bool transcribe(const String &host, const int16_t *samples, size_t sampleCount,
                    uint32_t sampleRate, String &transcript) {
        transcript = "";
        if (host.isEmpty() || !samples || sampleCount == 0) return false;

        WiFiClient connection;
        if (!connection.connect(host.c_str(), port_, 5000)) {
            Serial.printf("[Wyoming] Connection to %s:%u failed\n", host.c_str(), port_);
            return false;
        }
        // connect() resets the timeout, so it must be set afterwards (in seconds)
        connection.setTimeout(RESPONSE_TIMEOUT_S);

        String responseType;
        if (!sendLine(connection, "{\"type\":\"describe\",\"data\":{}}\n") ||
            !readEvent(connection, responseType) || responseType != "info") {
            Serial.println("[Wyoming] Server did not return an info event.");
            connection.stop();
            return false;
        }

        char event[192];
        snprintf(event, sizeof(event),
                 "{\"type\":\"transcribe\",\"data\":{}}\n"
                 "{\"type\":\"audio-start\",\"data\":{\"rate\":%lu,\"width\":2,\"channels\":1}}\n",
                 static_cast<unsigned long>(sampleRate));
        if (!sendLine(connection, event)) {
            Serial.println("[Wyoming] Could not start transcription.");
            connection.stop();
            return false;
        }

        const int32_t peak = pcmPeak(samples, sampleCount);
        const float gain = pcmGainForPeak(peak);
        Serial.printf("[Wyoming] Sending %u samples, peak %ld, gain %.2fx\n",
                      (unsigned)sampleCount, (long)peak, gain);

        int16_t chunk[CHUNK_SAMPLES];
        for (size_t offset = 0; offset < sampleCount; offset += CHUNK_SAMPLES) {
            const size_t count = min(static_cast<size_t>(CHUNK_SAMPLES), sampleCount - offset);
            for (size_t i = 0; i < count; ++i) {
                chunk[i] = static_cast<int16_t>(constrain(lroundf(samples[offset + i] * gain), -32768L, 32767L));
            }
            const size_t bytes = count * sizeof(int16_t);
            snprintf(event, sizeof(event),
                     "{\"type\":\"audio-chunk\",\"data\":{\"rate\":%lu,\"width\":2,\"channels\":1},\"payload_length\":%u}\n",
                     static_cast<unsigned long>(sampleRate), static_cast<unsigned>(bytes));
            if (!sendLine(connection, event) ||
                !sendBytes(connection, reinterpret_cast<const uint8_t *>(chunk), bytes)) {
                Serial.println("[Wyoming] PCM transfer failed.");
                connection.stop();
                return false;
            }
        }

        if (!sendLine(connection, "{\"type\":\"audio-stop\",\"data\":{}}\n")) {
            Serial.println("[Wyoming] Could not finish audio stream.");
            connection.stop();
            return false;
        }

        const unsigned long started = millis();
        bool received = false;
        while (millis() - started < RESPONSE_TIMEOUT_S * 1000UL &&
               (connection.connected() || connection.available() > 0)) {
            if (!readEvent(connection, responseType, &transcript)) break;
            if (responseType == "transcript") {
                received = true;
                break;
            }
            if (responseType == "error") break;
        }
        connection.stop();

        if (!received) {
            Serial.println("[Wyoming] No transcript received.");
            return false;
        }
        Serial.printf("[Wyoming] Transcript: %s\n", transcript.c_str());
        return true;
    }

private:
    static constexpr size_t CHUNK_SAMPLES = 512;
    static constexpr uint32_t RESPONSE_TIMEOUT_S = 60;

    Preferences prefs;
    uint16_t port_ = DEFAULT_PORT;

    static bool sendBytes(WiFiClient &connection, const uint8_t *data, size_t length) {
        size_t sent = 0;
        while (sent < length) {
            const size_t count = connection.write(data + sent, length - sent);
            if (count == 0) return false;
            sent += count;
        }
        return true;
    }

    static bool sendLine(WiFiClient &connection, const char *line) {
        return sendBytes(connection, reinterpret_cast<const uint8_t *>(line), strlen(line));
    }

    // Reads one event; the text of a transcript may be in the header or in the separate data block
    static bool readEvent(WiFiClient &connection, String &type, String *transcript = nullptr) {
        const String line = connection.readStringUntil('\n');
        if (line.isEmpty()) return false;

        JsonDocument event;
        if (deserializeJson(event, line)) return false;
        type = event["type"] | "";
        if (transcript) *transcript = event["data"]["text"] | "";

        const size_t dataLength = event["data_length"] | 0;
        const size_t payloadLength = event["payload_length"] | 0;
        const bool keepData = transcript && dataLength <= 4096;
        String dataBlock;
        uint8_t buffer[128];
        size_t remaining = dataLength;
        while (remaining > 0) {
            const size_t count = min(remaining, sizeof(buffer));
            if (connection.readBytes(buffer, count) != count) return false;
            if (keepData) dataBlock.concat(reinterpret_cast<const char *>(buffer), count);
            remaining -= count;
        }
        if (keepData && !dataBlock.isEmpty()) {
            JsonDocument extra;
            if (!deserializeJson(extra, dataBlock)) {
                const char *text = extra["text"];
                if (text) *transcript = text;
            }
        }

        remaining = payloadLength;
        while (remaining > 0) {
            const size_t count = min(remaining, sizeof(buffer));
            if (connection.readBytes(buffer, count) != count) return false;
            remaining -= count;
        }
        return !type.isEmpty();
    }
};
