#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <HTTPClient.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include "tone_wav.h"
#include "pcm_gain.h"
#include "log_time.h"
#include "language.h"

class MqttManager {
public:
    using CommandHandler = void (*)(const String &, const String &);
    using StationListProvider = String (*)(); // returns a JSON array of station names, e.g. ["A","B"]
    using SdListProvider = String (*)(); // returns a JSON array of SD file paths

private:
    WiFiClient netClient;
    PubSubClient client;
    Preferences prefs;
    CommandHandler handler = nullptr;
    StationListProvider stationListProvider = nullptr;
    SdListProvider sdListProvider = nullptr;
    String host;
    String user;
    String password;
    uint16_t port = 1883;
    String base = "esp32radio";
    String wavTransport = "mqtt";
    String sendWavTopic = "esp32radio/audio/upload";
    String sendWavUrl;
    class PcmWavStream : public Stream {
        const int16_t *samples = nullptr;
        size_t sampleCount = 0;
        size_t bytePosition = 0;
        float gain = 1.0f;
        uint8_t header[44] = {};
        size_t totalBytes = 0;

        uint8_t byteAt(size_t position) const {
            if (position < sizeof(header)) return header[position];
            const size_t pcmPosition = position - sizeof(header);
            const size_t sampleIndex = pcmPosition / sizeof(int16_t);
            const int32_t scaled = lroundf(samples[sampleIndex] * gain);
            const uint16_t sample = static_cast<uint16_t>(static_cast<int16_t>(constrain(scaled, -32768L, 32767L)));
            return static_cast<uint8_t>((sample >> ((pcmPosition % sizeof(int16_t)) * 8)) & 0xff);
        }

    public:
        void reset(const int16_t *pcm, size_t count, uint32_t sampleRate) {
            samples = pcm;
            sampleCount = count;
            bytePosition = 0;
            const int32_t peak = pcmPeak(samples, sampleCount);
            gain = pcmGainForPeak(peak);
            makeWavHeader(header, sampleCount, sampleRate);
            totalBytes = sizeof(header) + sampleCount * sizeof(int16_t);
            Serial.printf("[AUDIO] WAV RAM stream: Peak level %ld, Gain %.2fx, %u bytes\n",
              (long)peak, gain, (unsigned)totalBytes);
        }

        size_t size() const { return totalBytes; }
        size_t write(uint8_t) override { return 0; }
        int available() override { return static_cast<int>(totalBytes - bytePosition); }
        int read() override {
            if (bytePosition >= totalBytes) return -1;
            return byteAt(bytePosition++);
        }
        int peek() override {
            return bytePosition < totalBytes ? byteAt(bytePosition) : -1;
        }
        size_t readBytes(char *buffer, size_t length) override {
            const size_t count = min(length, totalBytes - bytePosition);
            for (size_t i = 0; i < count; ++i) buffer[i] = static_cast<char>(byteAt(bytePosition++));
            return count;
        }
    } wavStream;
    size_t wavPublishRemaining = 0;
    bool wavPublishActive = false;
    bool wavPublishOk = false;
    unsigned long lastAttempt = 0;
    bool attempted = false;
    bool discoverySent = false;

    String getMacId() const {
        char buf[9];
        snprintf(buf, sizeof(buf), "%08X", (uint32_t)ESP.getEfuseMac());
        return String(buf);
    }

    String topic(const char *suffix) const { return base + "/" + suffix; }

    void publishDiscovery() {
        String macId = getMacId();
        String prefix = base + "_" + macId;
        String device = "{\"identifiers\":[\"" + prefix + "\"],\"name\":\"ESP32 Radio (" + base + ")\",\"manufacturer\":\"DIY\",\"model\":\"ESP32-S3 MAX98357A\"}";
        bool availabilityOk = client.publish((base + "/availability").c_str(), "online", true);
        String shared = "\"availability_topic\":\"" + topic("availability") + "\",\"payload_available\":\"online\",\"payload_not_available\":\"offline\",\"device\":" + device;
        String switchPayload = String("{\"name\":\"") + HA_RADIO_POWER + "\",\"unique_id\":\"" + prefix + "_power\",\"command_topic\":\"" + topic("set/power") +
            "\",\"state_topic\":\"" + topic("state/power") + "\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"state_on\":\"ON\",\"state_off\":\"OFF\"," + shared + "}";
        String volumePayload = String("{\"name\":\"") + HA_RADIO_VOLUME + "\",\"unique_id\":\"" + prefix + "_volume\",\"command_topic\":\"" + topic("set/volume") +
            "\",\"state_topic\":\"" + topic("state/volume") + "\",\"min\":0,\"max\":21,\"step\":1," + shared + "}";
        String stationPayload = String("{\"name\":\"") + HA_RADIO_STATION + "\",\"unique_id\":\"" + prefix + "_station\",\"state_topic\":\"" + topic("state/station") + "\"," + shared + "}";
        String titlePayload = String("{\"name\":\"") + HA_RADIO_TITLE + "\",\"unique_id\":\"" + prefix + "_title\",\"state_topic\":\"" + topic("state/title") + "\"," + shared + "}";
        String alarmSwitchPayload = String("{\"name\":\"") + HA_ALARM_ENABLED + "\",\"unique_id\":\"" + prefix + "_alarm_enabled\",\"command_topic\":\"" + topic("set/alarm_enabled") +
            "\",\"state_topic\":\"" + topic("state/alarm_enabled") + "\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"state_on\":\"ON\",\"state_off\":\"OFF\"," + shared + "}";
        String alarmTimePayload = String("{\"name\":\"") + HA_ALARM_TIME + "\",\"unique_id\":\"" + prefix + "_alarm_time\",\"command_topic\":\"" + topic("set/alarm_time") +
            "\",\"state_topic\":\"" + topic("state/alarm_time") + "\"," + shared + "}";
        String alarmVolumePayload = String("{\"name\":\"") + HA_ALARM_VOLUME + "\",\"unique_id\":\"" + prefix + "_alarm_volume\",\"command_topic\":\"" + topic("set/alarm_volume") +
            "\",\"state_topic\":\"" + topic("state/alarm_volume") + "\",\"min\":0,\"max\":21,\"step\":1," + shared + "}";
        String alarmSourcePayload = String("{\"name\":\"") + HA_ALARM_SOURCE + "\",\"unique_id\":\"" + prefix + "_alarm_source\",\"command_topic\":\"" + topic("set/alarm_source") +
            "\",\"state_topic\":\"" + topic("state/alarm_source") + "\",\"options\":[\"radio\",\"sd\"]," + shared + "}";
        String announceVolPayload = String("{\"name\":\"") + HA_ANNOUNCE_VOLUME + "\",\"unique_id\":\"" + prefix + "_announce_volume\",\"command_topic\":\"" + topic("set/announce_volume") +
            "\",\"state_topic\":\"" + topic("state/announce_volume") + "\",\"min\":0,\"max\":21,\"step\":1," + shared + "}";
        String announceStatusPayload = String("{\"name\":\"") + HA_ANNOUNCE_STATUS + "\",\"unique_id\":\"" + prefix + "_announcing\",\"state_topic\":\"" + topic("state/announcing") +
            "\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\"," + shared + "}";
        bool discoveryOk = client.publish(("homeassistant/switch/" + prefix + "_power/config").c_str(), switchPayload.c_str(), true);
        discoveryOk = client.publish(("homeassistant/number/" + prefix + "_volume/config").c_str(), volumePayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/number/" + prefix + "_announce_volume/config").c_str(), announceVolPayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/binary_sensor/" + prefix + "_announcing/config").c_str(), announceStatusPayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/sensor/" + prefix + "_station/config").c_str(), stationPayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/sensor/" + prefix + "_title/config").c_str(), titlePayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/switch/" + prefix + "_alarm_enabled/config").c_str(), alarmSwitchPayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/time/" + prefix + "_alarm_time/config").c_str(), alarmTimePayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/number/" + prefix + "_alarm_volume/config").c_str(), alarmVolumePayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/select/" + prefix + "_alarm_source/config").c_str(), alarmSourcePayload.c_str(), true) && discoveryOk;
        if (stationListProvider) {
            String stationSelectPayload = String("{\"name\":\"") + HA_STATION_SELECT + "\",\"unique_id\":\"" + prefix + "_station_select\",\"command_topic\":\"" + topic("set/station_name") +
                "\",\"state_topic\":\"" + topic("state/station") + "\",\"options\":" + stationListProvider() + "," + shared + "}";
            discoveryOk = client.publish(("homeassistant/select/" + prefix + "_station_select/config").c_str(), stationSelectPayload.c_str(), true) && discoveryOk;
        }
        if (sdListProvider) {
            String sdOptions = sdListProvider();
            if (sdOptions != "[]") {
                String sdSelectPayload = String("{\"name\":\"") + HA_SD_SELECT + "\",\"unique_id\":\"" + prefix + "_sd_select\",\"command_topic\":\"" + topic("set/sd_play") +
                    "\",\"state_topic\":\"" + topic("state/sd_current") + "\",\"options\":" + sdOptions + "," + shared + "}";
                discoveryOk = client.publish(("homeassistant/select/" + prefix + "_sd_select/config").c_str(), sdSelectPayload.c_str(), true) && discoveryOk;
                String alarmSdSelectPayload = String("{\"name\":\"") + HA_ALARM_SD_FILE + "\",\"unique_id\":\"" + prefix + "_alarm_sd_path\",\"command_topic\":\"" + topic("set/alarm_sd_path") +
                    "\",\"state_topic\":\"" + topic("state/alarm_sd_path") + "\",\"options\":" + sdOptions + "," + shared + "}";
                discoveryOk = client.publish(("homeassistant/select/" + prefix + "_alarm_sd_path/config").c_str(), alarmSdSelectPayload.c_str(), true) && discoveryOk;
            }
        }
        Serial.printf("[MQTT] Discovery availability=%s config=%s (%u bytes)\n",
                      availabilityOk ? "OK" : "ERROR", discoveryOk ? "OK" : "ERROR", switchPayload.length());
        discoverySent = true;
    }

    void abortWavPublish(const char *reason) {
        wavPublishRemaining = 0;
        wavPublishActive = false;
        wavPublishOk = false;
        netClient.stop();
        Serial.printf("[%s] [MQTT] WAV transfer aborted: %s\n", logTimestamp().c_str(), reason);
    }

    void serviceWavPublish() {
        if (!wavPublishActive) return;

        uint8_t buffer[1024];
        const size_t blockSize = min(wavPublishRemaining, sizeof(buffer));
        const size_t bytesRead = wavStream.readBytes(reinterpret_cast<char *>(buffer), blockSize);
        if (bytesRead != blockSize) {
            abortWavPublish("Read error from recording buffer");
            return;
        }
        if (netClient.write(buffer, bytesRead) != bytesRead) {
            abortWavPublish("MQTT write failed");
            return;
        }

        wavPublishRemaining -= bytesRead;
        if (wavPublishRemaining == 0) {
            wavPublishActive = false;
            wavPublishOk = true;
            Serial.printf("[%s] [MQTT] WAV transfer completed (%s)\n", logTimestamp().c_str(), sendWavTopic.c_str());
        }
    }

    bool postWav() {
        if (!sendWavUrl.startsWith("http://")) {
            Serial.println("[HTTP] WAV upload aborted: A valid URL is required.");
            return false;
        }
        if (!wavStream.size()) return false;

        WiFiClient networkClient;
        HTTPClient http;
        if (!http.begin(networkClient, sendWavUrl)) {
            Serial.println("[HTTP] WAV upload aborted: Invalid URL.");
            return false;
        }

        http.setConnectTimeout(5000);
        http.setTimeout(15000);
        http.addHeader("Content-Type", "audio/wav");
        const int status = http.sendRequest("POST", &wavStream, wavStream.size());
        http.end();
        const bool success = status >= 200 && status < 300;
        Serial.printf("[HTTP] WAV-Upload %s, Status %d (%s)\n",
                      success ? "success" : "failed", status, sendWavUrl.c_str());
        return success;
    }

public:
    MqttManager() : client(netClient) {}

    void setStationListProvider(StationListProvider provider) { stationListProvider = provider; }
    void setSdListProvider(SdListProvider provider) { sdListProvider = provider; }

    // Republishes the discovery configs, e.g. after changes to the station or SD list
    void refreshDiscovery() {
        if (client.connected()) publishDiscovery();
    }

    void begin(CommandHandler commandHandler) {
        handler = commandHandler;
        prefs.begin("mqtt_config", true);
        host = prefs.getString("host", "");
        port = prefs.getUShort("port", 1883);
        user = prefs.getString("user", "");
        password = prefs.getString("pass", "");
        base = prefs.getString("base", "esp32radio-" + getMacId()); // unique per device, prevents topic collisions between several radios
        wavTransport = prefs.getString("wavTransport", "mqtt");
        sendWavTopic = prefs.getString("sendWav", "esp32radio/audio/upload");
        sendWavUrl = prefs.getString("sendWavUrl", "");
        prefs.end();
        if (!host.isEmpty()) client.setServer(host.c_str(), port);
        client.setCallback([this](char *topicName, byte *payload, unsigned int length) {
            String body;
            body.reserve(length + 1);
            for (unsigned int i = 0; i < length; ++i) body += (char)payload[i];
            if (handler) handler(String(topicName), body);
        });
        // Discovery payloads with favorite/SD lists are much larger than 1 KB
        client.setBufferSize(8192);
    }

    void saveConfig(const String &newHost, uint16_t newPort, const String &newUser, const String &newPassword,
                    const String &newBase, const String &newSendWavTopic,
                    const String &newWavTransport, const String &newSendWavUrl) {
        host = newHost;
        port = newPort;
        user = newUser;
        password = newPassword;
        base = newBase.isEmpty() ? "esp32radio" : newBase;
        wavTransport = newWavTransport == "post" || newWavTransport == "wyoming" ? newWavTransport : "mqtt";
        sendWavTopic = newSendWavTopic.isEmpty() ? "esp32radio/audio/upload" : newSendWavTopic;
        sendWavUrl = newSendWavUrl;
        prefs.begin("mqtt_config", false);
        prefs.putString("host", host);
        prefs.putUShort("port", port);
        prefs.putString("user", user);
        prefs.putString("pass", password);
        prefs.putString("base", base);
        prefs.putString("wavTransport", wavTransport);
        prefs.putString("sendWav", sendWavTopic);
        prefs.putString("sendWavUrl", sendWavUrl);
        prefs.end();
        if (!host.isEmpty()) client.setServer(host.c_str(), port);
        client.disconnect();
        discoverySent = false;
    }

    bool configured() const { return !host.isEmpty(); }
    const String &hostName() const { return host; }
    const String &wavTransportName() const { return wavTransport; }
    bool connected() { return client.connected(); }
    String getStatusJson() {
        String status = !configured() ? "disabled" : (client.connected() ? "connected" : "disconnected");
        return String("{\"status\":\"") + status + "\",\"host\":\"" + host + "\",\"port\":" + port + "}";
    }
    String getConfigJson() const {
        JsonDocument config;
        config["host"] = host;
        config["port"] = port;
        config["user"] = user;
        config["base"] = base;
        config["wavTransport"] = wavTransport;
        config["sendWav"] = sendWavTopic;
        config["sendWavUrl"] = sendWavUrl;
        String json;
        serializeJson(config, json);
        return json;
    }

    bool sendWav(const int16_t *samples, size_t sampleCount, uint32_t sampleRate) {
        if (!samples || sampleCount == 0 || sampleCount > sampleRate * 10 || wavPublishActive) return false;
        wavStream.reset(samples, sampleCount, sampleRate);
        if (wavTransport == "post") return postWav();
        if (wavTransport == "wyoming") return false;
        if (!client.connected() || sendWavTopic.isEmpty()) return false;
        if (sendWavTopic.indexOf('+') >= 0 || sendWavTopic.indexOf('#') >= 0) return false;

        const size_t fileSize = wavStream.size();
        const size_t topicLength = sendWavTopic.length();
        // PubSubClient::beginPublish limits the packet length to 16 bits,
        // so the MQTT PUBLISH header is constructed manually. 
        uint8_t header[1 + 4 + 2];
        size_t headerLength = 0;
        size_t remainingLength = 2 + topicLength + fileSize;
        header[headerLength++] = 0x30;
        do {
            uint8_t digit = remainingLength & 0x7F;
            remainingLength >>= 7;
            if (remainingLength) digit |= 0x80;
            header[headerLength++] = digit;
        } while (remainingLength);
        header[headerLength++] = topicLength >> 8;
        header[headerLength++] = topicLength & 0xFF;
        if (netClient.write(header, headerLength) != headerLength ||
            netClient.write(reinterpret_cast<const uint8_t *>(sendWavTopic.c_str()), topicLength) != topicLength) {
            abortWavPublish("MQTT header could not be sent");
            return false;
        }

        wavPublishRemaining = fileSize;
        wavPublishActive = true;
        wavPublishOk = false;
        Serial.printf("[%s] [MQTT] WAV transfer started: %s (%u Bytes)\n", logTimestamp().c_str(), sendWavTopic.c_str(), (unsigned)fileSize);
        // Send in one go: other publishes between blocks would corrupt the MQTT packet
        while (wavPublishActive) {
            serviceWavPublish();
            yield();
        }
        return wavPublishOk;
    }

    bool wavSending() const { return wavPublishActive; }

    void loop() {
        if (wavPublishActive && (!configured() || WiFi.status() != WL_CONNECTED || !client.connected())) {
            abortWavPublish("Connection lost");
        }
        if (!configured() || WiFi.status() != WL_CONNECTED) return;
        if (!client.connected()) {
            if (attempted && millis() - lastAttempt < 5000) return;
            attempted = true;
            lastAttempt = millis();
            String clientId = base + "-" + getMacId();
            bool ok = user.isEmpty() ? client.connect(clientId.c_str(), topic("availability").c_str(), 0, true, "offline") :
                client.connect(clientId.c_str(), user.c_str(), password.c_str(), topic("availability").c_str(), 0, true, "offline");
            if (!ok) {
                Serial.printf("[MQTT] Connection to %s:%u failed, state=%d\n", host.c_str(), port, client.state());
                return;
            }
            Serial.printf("[MQTT] Connected to %s:%u, base=%s\n", host.c_str(), port, base.c_str());
            client.subscribe(topic("set/#").c_str());
            publishDiscovery();
            Serial.println("[MQTT] Home Assistant discovery published.");
        }
        client.loop();
        if (!client.connected() && wavPublishActive) {
            abortWavPublish("MQTT connection lost");
            return;
        }
        serviceWavPublish();
    }

    void publishState(bool playing, const String &station, const String &title, int volume, int announceVolume = -1, bool announcing = false) {
        if (!client.connected()) return;
        client.publish(topic("state").c_str(), playing ? "playing" : "idle", true);
        client.publish(topic("state/power").c_str(), playing ? "ON" : "OFF", true);
        client.publish(topic("state/station").c_str(), station.c_str(), true);
        client.publish(topic("state/title").c_str(), title.c_str(), true);
        client.publish(topic("state/volume").c_str(), String(volume).c_str(), true);
        if (announceVolume >= 0) {
            client.publish(topic("state/announce_volume").c_str(), String(announceVolume).c_str(), true);
        }
        client.publish(topic("state/announcing").c_str(), announcing ? "ON" : "OFF", true);
    }

    void publishAlarmState(bool enabled, int hour, int minute, int volume, const String &source, const String &sdPath) {
        if (!client.connected()) return;
        char buf[9];
        snprintf(buf, sizeof(buf), "%02d:%02d:00", hour, minute);
        client.publish(topic("state/alarm_enabled").c_str(), enabled ? "ON" : "OFF", true);
        client.publish(topic("state/alarm_time").c_str(), buf, true);
        client.publish(topic("state/alarm_volume").c_str(), String(volume).c_str(), true);
        client.publish(topic("state/alarm_source").c_str(), source.c_str(), true);
        client.publish(topic("state/alarm_sd_path").c_str(), sdPath.c_str(), true);
    }

    void publish(const String &suffix, const String &payload, bool retained = true) {
        if (client.connected()) client.publish(topic(suffix.c_str()).c_str(), payload.c_str(), retained);
    }
};

extern MqttManager mqttMgr;
