#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>

class MqttManager {
public:
    using CommandHandler = void (*)(const String &, const String &);
    using StationListProvider = String (*)(); // liefert JSON-Array der Sendernamen, z.B. ["A","B"]
    using SdListProvider = String (*)(); // liefert JSON-Array der SD-Dateipfade

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
        String switchPayload = String("{\"name\":\"Radio Power\",\"unique_id\":\"") + prefix + "_power\",\"command_topic\":\"" + topic("set/power") +
            "\",\"state_topic\":\"" + topic("state/power") + "\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"state_on\":\"ON\",\"state_off\":\"OFF\"," + shared + "}";
        String volumePayload = String("{\"name\":\"Radio Lautstärke\",\"unique_id\":\"") + prefix + "_volume\",\"command_topic\":\"" + topic("set/volume") +
            "\",\"state_topic\":\"" + topic("state/volume") + "\",\"min\":0,\"max\":21,\"step\":1," + shared + "}";
        String stationPayload = String("{\"name\":\"Radio Sender\",\"unique_id\":\"") + prefix + "_station\",\"state_topic\":\"" + topic("state/station") + "\"," + shared + "}";
        String titlePayload = String("{\"name\":\"Radio Titel\",\"unique_id\":\"") + prefix + "_title\",\"state_topic\":\"" + topic("state/title") + "\"," + shared + "}";
        String alarmSwitchPayload = String("{\"name\":\"Wecker Aktiv\",\"unique_id\":\"") + prefix + "_alarm_enabled\",\"command_topic\":\"" + topic("set/alarm_enabled") +
            "\",\"state_topic\":\"" + topic("state/alarm_enabled") + "\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"state_on\":\"ON\",\"state_off\":\"OFF\"," + shared + "}";
        String alarmTimePayload = String("{\"name\":\"Weckzeit\",\"unique_id\":\"") + prefix + "_alarm_time\",\"command_topic\":\"" + topic("set/alarm_time") +
            "\",\"state_topic\":\"" + topic("state/alarm_time") + "\"," + shared + "}";
        String alarmVolumePayload = String("{\"name\":\"Wecker Lautstärke\",\"unique_id\":\"") + prefix + "_alarm_volume\",\"command_topic\":\"" + topic("set/alarm_volume") +
            "\",\"state_topic\":\"" + topic("state/alarm_volume") + "\",\"min\":0,\"max\":21,\"step\":1," + shared + "}";
        String alarmSourcePayload = String("{\"name\":\"Wecker Quelle\",\"unique_id\":\"") + prefix + "_alarm_source\",\"command_topic\":\"" + topic("set/alarm_source") +
            "\",\"state_topic\":\"" + topic("state/alarm_source") + "\",\"options\":[\"radio\",\"sd\"]," + shared + "}";
        String announceVolPayload = String("{\"name\":\"Durchsage Lautstärke\",\"unique_id\":\"") + prefix + "_announce_volume\",\"command_topic\":\"" + topic("set/announce_volume") +
            "\",\"state_topic\":\"" + topic("state/announce_volume") + "\",\"min\":0,\"max\":21,\"step\":1," + shared + "}";
        String announceStatusPayload = String("{\"name\":\"Durchsage Status\",\"unique_id\":\"") + prefix + "_announcing\",\"state_topic\":\"" + topic("state/announcing") +
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
            String stationSelectPayload = String("{\"name\":\"Radio Sender Auswahl\",\"unique_id\":\"") + prefix + "_station_select\",\"command_topic\":\"" + topic("set/station_name") +
                "\",\"state_topic\":\"" + topic("state/station") + "\",\"options\":" + stationListProvider() + "," + shared + "}";
            discoveryOk = client.publish(("homeassistant/select/" + prefix + "_station_select/config").c_str(), stationSelectPayload.c_str(), true) && discoveryOk;
        }
        if (sdListProvider) {
            String sdOptions = sdListProvider();
            if (sdOptions != "[]") {
                String sdSelectPayload = String("{\"name\":\"SD Datei Auswahl\",\"unique_id\":\"") + prefix + "_sd_select\",\"command_topic\":\"" + topic("set/sd_play") +
                    "\",\"state_topic\":\"" + topic("state/sd_current") + "\",\"options\":" + sdOptions + "," + shared + "}";
                discoveryOk = client.publish(("homeassistant/select/" + prefix + "_sd_select/config").c_str(), sdSelectPayload.c_str(), true) && discoveryOk;
                String alarmSdSelectPayload = String("{\"name\":\"Wecker SD-Datei\",\"unique_id\":\"") + prefix + "_alarm_sd_path\",\"command_topic\":\"" + topic("set/alarm_sd_path") +
                    "\",\"state_topic\":\"" + topic("state/alarm_sd_path") + "\",\"options\":" + sdOptions + "," + shared + "}";
                discoveryOk = client.publish(("homeassistant/select/" + prefix + "_alarm_sd_path/config").c_str(), alarmSdSelectPayload.c_str(), true) && discoveryOk;
            }
        }
        Serial.printf("[MQTT] Discovery availability=%s config=%s (%u Bytes)\n",
                      availabilityOk ? "OK" : "FEHLER", discoveryOk ? "OK" : "FEHLER", switchPayload.length());
        discoverySent = true;
    }

public:
    MqttManager() : client(netClient) {}

    void setStationListProvider(StationListProvider provider) { stationListProvider = provider; }
    void setSdListProvider(SdListProvider provider) { sdListProvider = provider; }

    // Republished die Discovery-Configs erneut, z.B. nach Aenderungen an der Sender- oder SD-Liste
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
        base = prefs.getString("base", "esp32radio-" + getMacId()); // eindeutig je Gerät, verhindert Topic-Kollisionen bei mehreren Radios
        prefs.end();
        if (!host.isEmpty()) client.setServer(host.c_str(), port);
        client.setCallback([this](char *topicName, byte *payload, unsigned int length) {
            String body;
            body.reserve(length + 1);
            for (unsigned int i = 0; i < length; ++i) body += (char)payload[i];
            if (handler) handler(String(topicName), body);
        });
        // Discovery-Payloads mit Favoriten-/SD-Listen sind deutlich groesser als 1 KB
        client.setBufferSize(8192);
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
            if (attempted && millis() - lastAttempt < 5000) return;
            attempted = true;
            lastAttempt = millis();
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
