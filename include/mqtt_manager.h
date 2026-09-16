#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>

class MqttManager {
public:
    using CommandHandler = void (*)(const String &, const String &);

private:
    WiFiClient netClient;
    PubSubClient client;
    Preferences prefs;
    CommandHandler handler = nullptr;
    String host;
    String user;
    String password;
    uint16_t port = 1883;
    String base = "esp32radio";
    unsigned long nextAttempt = 0;
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
        String switchPayload = String("{\"name\":\"Radio Power\",\"unique_id\":\"") + prefix + "_power\",\"command_topic\":\"" + topic("set/power") +
            "\",\"state_topic\":\"" + topic("state/power") + "\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"state_on\":\"ON\",\"state_off\":\"OFF\"," + shared + "}";
        String volumePayload = String("{\"name\":\"Radio Lautstärke\",\"unique_id\":\"") + prefix + "_volume\",\"command_topic\":\"" + topic("set/volume") +
            "\",\"state_topic\":\"" + topic("state/volume") + "\",\"min\":0,\"max\":21,\"step\":1," + shared + "}";
        String stationPayload = String("{\"name\":\"Radio Sender\",\"unique_id\":\"") + prefix + "_station\",\"state_topic\":\"" + topic("state/station") + "\"," + shared + "}";
        String titlePayload = String("{\"name\":\"Radio Titel\",\"unique_id\":\"") + prefix + "_title\",\"state_topic\":\"" + topic("state/title") + "\"," + shared + "}";
        bool discoveryOk = client.publish(("homeassistant/switch/" + prefix + "_power/config").c_str(), switchPayload.c_str(), true);
        discoveryOk = client.publish(("homeassistant/number/" + prefix + "_volume/config").c_str(), volumePayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/sensor/" + prefix + "_station/config").c_str(), stationPayload.c_str(), true) && discoveryOk;
        discoveryOk = client.publish(("homeassistant/sensor/" + prefix + "_title/config").c_str(), titlePayload.c_str(), true) && discoveryOk;
        Serial.printf("[MQTT] Discovery availability=%s config=%s (%u Bytes)\n",
                      availabilityOk ? "OK" : "FEHLER", discoveryOk ? "OK" : "FEHLER", switchPayload.length());
        discoverySent = true;
    }

public:
    MqttManager() : client(netClient) {}

    void begin(CommandHandler commandHandler) {
        handler = commandHandler;
        prefs.begin("mqtt_config", true);
        host = prefs.getString("host", "");
        port = prefs.getUShort("port", 1883);
        user = prefs.getString("user", "");
        password = prefs.getString("pass", "");
        base = prefs.getString("base", "esp32radio");
        prefs.end();
        if (!host.isEmpty()) client.setServer(host.c_str(), port);
        client.setCallback([this](char *topicName, byte *payload, unsigned int length) {
            String body;
            body.reserve(length + 1);
            for (unsigned int i = 0; i < length; ++i) body += (char)payload[i];
            if (handler) handler(String(topicName), body);
        });
        client.setBufferSize(1024);
    }

    void saveConfig(const String &newHost, uint16_t newPort, const String &newUser, const String &newPassword, const String &newBase) {
        host = newHost;
        port = newPort;
        user = newUser;
        password = newPassword;
        base = newBase.isEmpty() ? "esp32radio" : newBase;
        prefs.begin("mqtt_config", false);
        prefs.putString("host", host);
        prefs.putUShort("port", port);
        prefs.putString("user", user);
        prefs.putString("pass", password);
        prefs.putString("base", base);
        prefs.end();
        client.setServer(host.c_str(), port);
        client.disconnect();
        discoverySent = false;
    }

    bool configured() const { return !host.isEmpty(); }
    bool connected() { return client.connected(); }
    String getStatusJson() {
        String status = !configured() ? "disabled" : (client.connected() ? "connected" : "disconnected");
        return String("{\"status\":\"") + status + "\",\"host\":\"" + host + "\",\"port\":" + port + "}";
    }
    String getConfigJson() const {
        return String("{\"host\":\"") + host + "\",\"port\":" + port + ",\"user\":\"" + user + "\",\"base\":\"" + base + "\"}";
    }

    void loop() {
        if (!configured() || WiFi.status() != WL_CONNECTED) return;
        if (!client.connected()) {
            if (millis() < nextAttempt) return;
            nextAttempt = millis() + 5000;
            String clientId = base + "-" + getMacId();
            bool ok = user.isEmpty() ? client.connect(clientId.c_str(), topic("availability").c_str(), 0, true, "offline") :
                client.connect(clientId.c_str(), user.c_str(), password.c_str(), topic("availability").c_str(), 0, true, "offline");
            if (!ok) {
                Serial.printf("[MQTT] Verbindung zu %s:%u fehlgeschlagen, state=%d\n", host.c_str(), port, client.state());
                return;
            }
            Serial.printf("[MQTT] Verbunden mit %s:%u, base=%s\n", host.c_str(), port, base.c_str());
            client.subscribe(topic("set/#").c_str());
            publishDiscovery();
            Serial.println("[MQTT] Home-Assistant-Discovery publiziert.");
        }
        client.loop();
    }

    void publishState(bool playing, const String &station, const String &title, int volume) {
        if (!client.connected()) return;
        client.publish(topic("state").c_str(), playing ? "playing" : "idle", true);
        client.publish(topic("state/power").c_str(), playing ? "ON" : "OFF", true);
        client.publish(topic("state/station").c_str(), station.c_str(), true);
        client.publish(topic("state/title").c_str(), title.c_str(), true);
        client.publish(topic("state/volume").c_str(), String(volume).c_str(), true);
    }

    void publish(const String &suffix, const String &payload, bool retained = true) {
        if (client.connected()) client.publish(topic(suffix.c_str()).c_str(), payload.c_str(), retained);
    }
};

extern MqttManager mqttMgr;
